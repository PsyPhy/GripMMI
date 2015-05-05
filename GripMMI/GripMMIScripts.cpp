///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

// The GRIP hardware uses a set of scripts to run the experiment. The scripts
// are organized into 'subjects', 'protocols', 'tasks' and 'steps'. GRIP sends 
// the current subject, protocol, task and step ID as part of the housekeeping
// telemetry packet. By using an exact copy of the scripts with the GripMMI
// one can follow the progress of the experiment on the ground. This file provides
// the methods to parse the script files and step through them. The so-called 
// script crawler part of the display will either be live, showing the current step indicated
// by GRIP housekeeping telemetry, or in manual mode where the GripMMI operator 
// can select the subject, protocol, task and step through ListBoxes and TextBoxes
// on the GripMMI GUI.

// NB Thorough knowledge of EPM and GRIP (DEX) interfaces is assumed.
// See https://github.com/PsyPhy/GripMMI/wiki, and the documents 
// "DEX-ICD-00383-QS Software Interface Control Document.pdf" 
//     and 
// "EPM-OHB-SP-0005_iss4 with remarks from Joe.pdf" in the Documentation directory.

#include "stdafx.h"
#include <Windows.h>

#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys\stat.h>
#include <conio.h>
#include <math.h>
#include <float.h>
#include <vcclr.h>

#include "GripMMIDesktop.h"
#include "..\Useful\Useful.h"
#include "..\Useful\VectorsMixin.h"
#include "..\Useful\fMessageBox.h"
#include "..\Useful\fOutputDebugString.h"
#include "..\Useful\ParseCommaDelimitedLine.h"

#include "..\Grip\GripPackets.h"
#include "..\Grip\DexAnalogMixin.h"

using namespace GripMMI;

// This module is built on top of existing code that allowed CADMOS and others
// to step through the GRIP scripts without having to run them on the GRIP hardware.
// Thus, it is built on some legacy code that preceded the conversion to VC2010. 
// Some functionality is therefore built on local static arrays, rather than dynamically
// allocated arrays in the GripMMI class, in a large part because VC2010 does not support
// 'mixed' data types (i.e. arrays of strings). This was an unexpected 'feature' (limitation) of VC2010.

// Relative or absolute paths to the script files are set at startup and kept here
// as global variables for access by various routines.
char scriptDirectory[MAX_PATHLENGTH];
char pictureFilenamePrefix[MAX_PATHLENGTH];

// The subject/session file defines the session file for each subject.
int  subjectID[MAX_MENU_ITEMS];
char session_file_list[MAX_MENU_ITEMS][MAX_MENU_ITEM_LENGTH];
int  nSubjects = 0;

// A protocol file contains a list protocols for a defined session.
char protocol_file_list[MAX_MENU_ITEMS][MAX_MENU_ITEM_LENGTH];
int  protocolID[MAX_MENU_ITEMS];
int  nProtocols = 0;

// A task file contains the list of tasks for a given protocol.
char task_file_list[MAX_MENU_ITEMS][MAX_MENU_ITEM_LENGTH];
int  taskID[MAX_MENU_ITEMS];
int	 nTasks = 0;			

// A task is a list of lines made up of steps and comments.
// Each step has an associated step ID and the picture and message that the GRIP
// subject will see at that point in the script execution.
const char *type[MAX_STEPS];
char picture[MAX_STEPS][MAX_PICTURE_LENGTH];
char message[MAX_STEPS][MAX_MESSAGE_LENGTH];
int  stepID[MAX_STEPS];
// The 'comment' array determines which lines are steps and which are comments.
// This is used to skip to the next actual step when the 'Next' button in pressed.
bool comment[MAX_STEPS];
// nSteps holds the number of lines, including comments, in the task script.
// It is not the number of executable steps. Therefore nSteps is a misnomer.
int nSteps = 0; 

// These are the 3 types of steps. 
const char *type_status = "Status (script will continue)";
const char *type_query =  "Query (waiting for response)";
const char *type_alert =  "Alert (seen only if error)";

// A convenient macro to hand message/picture pairs.
// It makes the parsing code more readable, but be careful how it is used.
// Improper use could lead to side effects that are hard to detect.
#define add_alert(y,z) { strcpy( message[lines], y ); if ( z ) strcpy( picture[lines], z ); else strcpy( picture[lines], "" ); type[lines] = type_alert; }

// Parse the specified task file, filling the GUI ListBox and the global tables
// 'message', 'picture', 'comment' and 'stepID'.
void GripMMIDesktop::ParseTaskFile ( const char *task_file ) {

	FILE *fp;

	// Count the number of lines in the text file / task list.;
	int lines;
	// Keep track of the most recent step, excluding comments.
	int current_step;

	// Buffers used to read in and parse the lines in the task file.
	int tokens;
	char *token[MAX_TOKENS];
	char line[2048];
	char status_picture[MAX_PICTURE_LENGTH] = "blank.bmp";
	char status_message[MAX_MESSAGE_LENGTH] = "";

	// A string used to create error messages.
	char msg[MAX_ERROR_MESSAGE_LENGTH];

	fp = fopen( task_file, "r" );
	if ( !fp ) {
		// Signal the error.
		sprintf( msg, "Error opening task file %s for read.\n", task_file );
		::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
		return;
	}

	// Clear the menu and reset the list lengths.
	stepList->Items->Clear();
	nSteps = 0;
	lines = 0;
	current_step = 0;

	// Create a dummy first line, which is a comment.
	stepList->Items->Add( "<Waiting to start ...>" );
	strcpy( message[lines], status_message );
	strcpy( picture[lines], status_picture );
	type[lines] = type_status;
	comment[lines] = true;
	stepID[lines] = current_step;
	lines++;

	// Step through each line in the file.
	// Read in the entire line as a character string each time.
	while ( fgets( line, sizeof( line ), fp ) && lines < MAX_STEPS ) {

		// Make sure the line is a null terminated string.
		// In the unlikely event that a line in the file exceeds the size of the line buffer,
		// the line will be truncated and a second line will be read. This will surely generate
		// an error in the parsing of the second line, so I don't treat the overflow explicitly here.
		line[strlen( line ) - 1] = 0;
		// Insert the entire line into the GUI list.
		String^ line_string = gcnew String( line );
		stepList->Items->Add( line_string );

		// Now parse the line according to the fields defined by the GRIP ICD.
		tokens = ParseCommaDelimitedLine( token, line );

		if ( tokens ) {

			// First we hand commands that change the status messag and picture.
			if ( !strcmp( token[0], "CMD_LOG_MESSAGE" ) ) {
				int value, items;
				// CMD_LOG_MESSAGE takes a 0 or a 1 as the second value,
				// but some scripts use the constants 'logmsg' (0) or 'usermsg' (1).
				// So here we treat those as a special case to set the value of the second parameter.
				if ( !strcmp( token[1], "logmsg" ) ) value = 0;
				else if ( !strcmp( token[1], "usermsg" ) ) value = 1;
				// If neither 'logmsg' or 'usermsg', then read a 0 or 1.
				// But if the sscanf fails (items == 0) then default to 0.
				else {
					items = sscanf( token[1], "%d", &value );
					value = items && value;
				}
				// Only user messages (value == 1) change the status message that is shown to the subject.
				// We ignore log messages.
				if ( value != 0 ) {
					if ( token[2] ) strcpy( status_message, token[2] );
					else strcpy( status_message, "" );
				}
			}
			else if ( !strcmp( token[0], "CMD_SET_PICTURE" ) ) {
				if ( tokens > 1 ) strcpy( status_picture, token[1] );
				else strcpy( status_picture, "blank.bmp" );
			}

			// Now interpret actual commands.
			// CMD_WAIT_SUBJ_READY generates a 'query'.
			if ( !strcmp( token[0], "CMD_WAIT_SUBJ_READY" ) ) {
				strcpy( message[lines], token[1] );
				strcpy( picture[lines], token[2] );
				type[lines] = type_query;
			}
			// The following commands generate 'alerts'.
			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_ATTARGET" ) ) add_alert( token[13], token[14] )		
			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_GRIP" ) ) add_alert( token[4], token[5] )
			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_GRIPFORCE" ) ) add_alert( token[11], token[12] )
			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_SLIP" ) ) add_alert( token[11], token[12] )
			else if ( !strcmp( token[0], "CMD_CHK_MASS_SELECTION" ) ) add_alert( "Put mass in cradle X and pick up mass from cradle Y.", "TakeMass.bmp" )
			else if ( !strcmp( token[0], "CMD_CHK_HW_CONFIG" ) ) add_alert( token[1], token[2] )
			else if ( !strcmp( token[0], "CMD_ALIGN_CODA" ) ) add_alert( token[1], token[2] )
			else if ( !strcmp( token[0], "CMD_CHK_CODA_ALIGNMENT" ) ) add_alert( token[4], token[5] )
			else if ( !strcmp( token[0], "CMD_CHK_CODA_FIELDOFVIEW" ) ) add_alert( token[9], token[10] )
			else if ( !strcmp( token[0], "CMD_CHK_CODA_PLACEMENT" ) ) add_alert( token[11], token[12] )
			else if ( !strcmp( token[0], "CMD_CHK_MOVEMENTS_AMPL" ) ) add_alert( token[6], token[7] )
			else if ( !strcmp( token[0], "CMD_CHK_MOVEMENTS_CYCLES" ) ) add_alert( token[7], token[8] )
			else if ( !strcmp( token[0], "CMD_CHK_START_POS" ) ) add_alert( token[7], token[8] )
			else if ( !strcmp( token[0], "CMD_CHK_MOVEMENTS_DIR" ) ) add_alert( token[6], token[7] )
			else if ( !strcmp( token[0], "CMD_CHK_COLLISIONFORCE" ) ) add_alert( token[4], token[5] )
			else if ( !strcmp( token[0], "CMD_CHK_MANIP_VISIBILITY" ) ) add_alert( token[3], token[4] )
			// If it wasn't a 'query' or an 'alert' then it was a 'status' command.
			else {
				strcpy( message[lines], status_message );
				strcpy( picture[lines], status_picture );
				type[lines] = type_status;
			}
			comment[lines] = false;
			current_step++;
			stepID[lines] = current_step;
		}
		// If there were no tokes, then the line is a comment line.
		// Use the the most recent status, picture and stepID and mark is as a comment.
		else {
			strcpy( message[lines], status_message );
			strcpy( picture[lines], status_picture );
			type[lines] = type_status;
			comment[lines] = true;
			stepID[lines] = current_step;
		}
		// Count the total number of lines.
		lines++;
	}
	fclose( fp );

	// nSteps holds the number of lines in the menu showing the task, including comments 
	// not the number of real steps. So nSteps is a misnomer.
	nSteps = lines;
	// Check for overrun of step buffer.
	if ( nSteps >= MAX_STEPS ) {
		// Signal the error, but allow execution to continue.
		sprintf( msg, "Number of lines in %s exceeds limit.\n", task_file );
		::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
	}
	else {
		// Show the end of the script in the GUI with a final comment line and status message.
		stepList->Items->Add( "************ End of Script ************" );
		strcpy( message[lines], "*********** End of Script ***********\n*********** End of Script ***********\n*********** End of Script ***********\n*********** End of Script ***********" );
		strcpy( picture[lines], "blank.bmp" );
		type[lines] = type_status;
		comment[lines] = true;
		stepID[lines] = 9999;
	}
}

// Parse the specified protocol file, filling the GUI ListBox 'taskList'
//  and the global table 'task_file'.
void GripMMIDesktop::ParseProtocolFile ( const char *protocol_file ) {

	FILE *fp;

	// Buffers to read in the lines in the protocol file.
	int tokens;
	char *token[MAX_TOKENS];
	char line[2048];

	// Count the number of lines read.
	int line_n = 0;

	// Initialize the list of tasks in the GUI to empty.
	taskList->Items->Clear();
	// Set the number of tasks to zero so that if we exit early
	// due to an error, the task list will be marked as empty.
	nTasks = 0;

	// Buffer for constructing error message.
	char msg[MAX_ERROR_MESSAGE_LENGTH];

	// We want to work in the same directory as the subject file.
	// Get the directory from the filename.
	char directory[MAX_PATHLENGTH];
	// Work backward until we find a backslash, if any.
	int i;
	for ( i = strlen( protocol_file ); i >=0; i-- ) {
		if ( protocol_file[i] == '\\' ) break;
	}
	// Copy what is before the backslash as the directory.
	// Make it a null-terminated string.
	strncpy( directory, protocol_file, i + 1 );
	directory[i+1] = 0;

	// Open the protocol file.
	fp = fopen( protocol_file, "r" );
	if ( !fp ) {
		// Signal the error.
		sprintf( msg, "Error opening protocol file %s for read.\n", protocol_file );
		::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
		return;
	}

	// Work our way through the protocol file line-by-line.
	while ( fgets( line, sizeof( line ), fp ) && nTasks <= MAX_MENU_ITEMS ) {

		// Split the line into tokens.
		tokens = ParseCommaDelimitedLine( token, line );

		// Lines in protocol files are divided into 4 fields.
		if ( tokens == 4 ) {
			// First parameter must be CMD_TASK.
			if ( strcmp( token[0], "CMD_TASK" ) ) {
				sprintf( msg, "%s Line %03d Command not CMD_TASK: %s\n", protocol_file, line_n, token[0] );
				::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
			}
			// Second parameter is a task ID. Should be an integer value.
			if ( 1 != sscanf( token[1], "%d", &taskID[nTasks] ) ) {
				sprintf( msg, "%s Line %03d Error reading task ID: %s\n", protocol_file, line_n, token[1] );
				::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
				exit( - 1 );
			}
			// The third item is the name of the protocol file.
			// Check if it is present and readable, unless told to ignore it.
			if ( !strstr( token[2], "ignore" ) ) {
				char task_filename[1024];
				strcpy( task_filename, directory );
				strcat( task_filename, token[2] );
				if ( _access( task_filename, 0x00 ) ) {
					sprintf( msg, "%s Line %03d Cannot access protocol file: %s\n", protocol_file, line_n, task_filename );
					::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
				}
				else {
					// Add the filename to the global list of protocols and to the GUI.
					strcpy( task_file_list[nTasks], task_filename );
					taskList->Items->Add( gcnew String( token[3] ) );
					nTasks++;
				}
			}
		}
		else if ( tokens != 0 ) {
			sprintf( msg, "%s Line %03d Wrong number of parameters: %s\n", protocol_file, line_n, line );
			::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
		}
		// If tokens is 0, then skip the line as a comment.
		else {}
	}
	fclose( fp );
	if ( nTasks >= MAX_MENU_ITEMS ) {
		// Signal the error, but allow execution to continue.
		sprintf( msg, "Number of items in %s exceeds limit.\n", protocol_file );
		::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
	}
}

// Parse the specified session file, filling the GUI ListBox 'protocolList'
//  and the global table 'protocol_file'.
// The parameter session_filename is assumed to be a null terminated string.
void GripMMIDesktop::ParseSessionFile ( const char *session_file ) {

	FILE *fp;

	// Buffer to hold a line from the file;
	char line[2048];
	// The line will be broken into tokens (fields).
	int tokens;
	char *token[MAX_TOKENS];

	// Count the number of lines read.
	int line_n = 0;

	// Buffer for constructing error messages.
	char msg[MAX_ERROR_MESSAGE_LENGTH];

	// Clear the list of protocols in the GUI ListBox.
	protocolList->Items->Clear();

	// Set the number of protocols to zero. So if we exit early
	// due to an error, the protocol list will be marked as empty.
	nProtocols = 0;

	// We want to work in the same directory as the subject file.
	// Get the directory from the filename.
	char directory[MAX_PATHLENGTH];
	// Work backward until we find a backslash, if any.
	int i;
	for ( i = strlen( session_file ); i >=0; i-- ) {
		if ( session_file[i] == '\\' ) break;
	}
	// Copy what is before the backslash as the directory.
	// Make it a null-terminated string.
	strncpy( directory, session_file, i + 1 );
	directory[i+1] = 0;

	// Open the session file, if we can.
	fp = fopen( session_file, "r" );
	if ( !fp ) {
		// Signal the error.
		sprintf( msg, "Error opening session file %s for read.\n", session_file );
		::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
		return;
	}

	// Work our way through the session file line-by-line.
	while ( fgets( line, sizeof( line ), fp ) && nProtocols < MAX_MENU_ITEMS ) {

		line_n++;
		tokens = ParseCommaDelimitedLine( token, line );

		// Lines in session files are divided into 4 fields.
		if ( tokens == 4 ) {
			// First parameter must be CMD_PROTOCOL.
			if ( strcmp( token[0], "CMD_PROTOCOL" ) ) {
				sprintf( msg, "%s Line %03d Command not CMD_PROTOCOL: %s\n", session_file, line_n, token[0] );
				::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
			}
			// Second parameter is a protocol ID. Should be an integer value.
			if ( 1 != sscanf( token[1], "%d", &protocolID[nProtocols] ) ) {
				sprintf( msg, "%s Line %03d Error reading protocol ID: %s\n", session_file, line_n, token[1] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				exit( - 1 );
			}
			// The third item is the name of the protocol file.
			// Check if it is present and readable, unless told to ignore it.
			if ( !strstr( token[2], "ignore" ) ) {
				char protocol_filename[MAX_PATHLENGTH];
				strcpy( protocol_filename, directory );
				strcat( protocol_filename, token[2] );
				if ( _access( protocol_filename, 0x00 ) ) {
					sprintf( msg, "%s Line %03d Cannot access protocol file: %s\n", session_file, line_n, protocol_filename );
					::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				}
				else {
					// Add the filename to the list of protocols.
					strcpy( protocol_file_list[nProtocols], protocol_filename );
					// SendDlgItemMessage( IDC_PROTOCOLS, LB_ADDSTRING, 0, (LPARAM) token[3] );
					protocolList->Items->Add( gcnew String( token[3] ) );
					nProtocols++;
				}
			}

		}
		else if ( tokens != 0 ) {
			sprintf( msg, "%s Line %03d Wrong number of parameters: %s\n", session_file, line_n, line );
			::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
			return;
		}
		// If tokens is 0, then skip the line as a comment.
		else {}
	}
	fclose( fp );
	if ( nProtocols >= MAX_MENU_ITEMS ) {
		// Signal the error, but allow execution to continue.
		sprintf( msg, "Number of items in %s exceeds limit.\n", session_file );
		::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
	}
}

// Parse the specified subject file, filling the GUI ListBox 'subjectList'
//  and the global table 'session_file'.
// The parameter subject_filename is assumed to be a null terminated string.
int GripMMIDesktop::ParseSubjectFile ( const char *subject_file ) {

	FILE *fp;

	// Buffer to hold a line from the file.
	char line[2048];
	// Each line will be broken into tokens (fields).
	int tokens;
	char *token[MAX_TOKENS];

	// Number of lines in the file.
	int line_n = 0;

	// Clear the list of protocols in the GUI ListBox.
	subjectList->Items->Clear();
	nSubjects = 0;

	// We want to work in the same directory as the subject file.
	// Get the directory from the filename.
	char directory[MAX_PATHLENGTH];
	// Work backward until we find a backslash, if any.
	int i;
	for ( i = strlen( subject_file ); i >=0; i-- ) {
		if ( subject_file[i] == '\\' ) break;
	}
	// Copy what is before the backslash as the directory.
	// Make it a null-terminated string.
	strncpy( directory, subject_file, i + 1 );
	directory[i+1] = 0;

	// Open the subject file, if we can.
	fp = fopen( subject_file, "r" );
	if ( !fp ) {
		char msg[1024];
		sprintf( msg, "Error opening subject file %s for read.", subject_file );
		printf( "%s\n", msg );
		::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
		return( ERROR_EXIT );
	}

	// Step through line by line and follow the links to the session files.
	while ( fgets( line, sizeof( line ), fp ) && nSubjects < MAX_MENU_ITEMS ) {

		// Count the lines in the file.
		line_n++;

		// Break the line into pieces as defined by the DEX/GRIP ICD.
		tokens = ParseCommaDelimitedLine( token, line );

		// Parse each line and do some syntax error checking.
		if ( tokens == 5 ) {

			// First parameter must be CMD_USER.
			if ( strcmp( token[0], "CMD_USER" ) ) {
				char msg[1024];
				sprintf( msg, "%s Line %03d Command not CMD_USER: %s\n", subject_file, line_n, token[0] );
				::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
				exit( ERROR_EXIT );
			}
			// Second parameter is a subject ID. Should be an integer value.
			// Verify also that subject IDs are unique.
			if ( 1 != sscanf( token[1], "%d", &subjectID[nSubjects] ) ) {
				char msg[1024];
				// Report error for invalid subject ID field.
				sprintf( msg, "%s Line %03d Error reading subject ID: %s\n", subject_file, line_n, token[1] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				exit( ERROR_EXIT );
			}
			// The third parameter is the name of the session file.
			// Here we check that it is present and if so, we process it as well.
			char session_filename[1024];
			strcpy( session_filename, directory );
			strcat( session_filename, token[3] );
			if ( _access( session_filename, 0x00 ) ) {
				char msg[1024];
				// The file must not only be present, it also has to be readable.
				// I had a funny problem with this at one point. Maybe MAC OS had created links.
				sprintf( msg, "%s Line %03d Cannot access session file: %s\n", subject_file, line_n, session_filename );
				::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
				exit( ERROR_EXIT );
			}	
			else {		
				strcpy( session_file_list[nSubjects], session_filename );
				subjectList->Items->Add( gcnew String( token[4] ));
				nSubjects++;	
			}
			// There is a fifth field to the line, which is text describing the subject.
			// For the moment I don't do any error checking on that parameter.
		}
		// If the number of tokens is anything but 0 or 5, it is an error.
		else if ( tokens != 0 ) {
			char msg[MAX_ERROR_MESSAGE_LENGTH];
			sprintf( msg, "%s Line %03d Wrong number of parameters: %s\n", subject_file, line_n, line );
			::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
			exit( ERROR_EXIT );
		}	
		// If the number of tokens is 0, it is a comment or blank line.
		else {}
	}

	fclose( fp );
	if ( nSubjects >= MAX_MENU_ITEMS ) {
		char msg[MAX_ERROR_MESSAGE_LENGTH];
		// Signal the error, but allow execution to continue.
		sprintf( msg, "Number of items in %s exceeds limit.\n", subject_file );
		::MessageBox( NULL, msg, "GRIP Script Crawler", MB_OK | MB_ICONERROR );
	}
	return( 0 );

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Given an index into the list of subjects, load the protocols from the associated session file 
//  and show the specified subject as the selected subject in the subjectList ListBox.
void GripMMIDesktop::GoToSpecifiedSubject( int subject ) 
{
	if ( subject < 0 || subject >= nSubjects ) {
		// If subject index is out of range, show no subject selected.
		subjectList->SelectedIndex = -1;
		subjectIDBox->Text = "";
		// Clear the protocol list to reflect that there is no subject selected.
		protocolList->Items->Clear();
		nProtocols = 0;
	}
	else {
		// Parse the session file associated with this subject.
		// This will reload the list of protocols stored in global variable protocol_file_list.
		ParseSessionFile( session_file_list[subject] );
		// Show which subject is selected in both the ListBox and TextBox of the GUI.
		subjectList->SelectedIndex = subject;
		subjectIDBox->Text = Convert::ToString( subjectID[subject] );
	}	
	// We don't know yet what protocol will be selected.
	GoToSpecifiedProtocol( UNDEFINED );
}

// Given an index into the current list of protocols, load the tasks from the specified protocol 
//  and show it as the selected protocol in the protocolList ListBox.
void GripMMIDesktop::GoToSpecifiedProtocol( int protocol ) 
{
	if ( protocol < 0 || protocol >= nProtocols ) {
		// If out of range, show no protocol selected.
		protocolList->SelectedIndex = -1;
		protocolIDBox->Text = "";
		// Clear the protocol list to reflect that there is no subject selected.
		taskList->Items->Clear();
		nTasks = 0;
	}
	else {
		// Parse the protocol file associated with the selected protocol.
		// This will reload the list of tasks stored in the global variable task_file_list.
		ParseProtocolFile( protocol_file_list[protocol] );
		// Show which protocol is selected in the ListBox and TextBox of the GUI.
		protocolList->SelectedIndex = protocol;
		protocolIDBox->Text = Convert::ToString( protocolID[protocol] );
	}	
	// We don't know yet what task will be selected.
	GoToSpecifiedTask( UNDEFINED );
}

// Given an index into the current list of tasks, load the steps from the specified task 
//  and show it as the selected task in the taskList ListBox.
void GripMMIDesktop::GoToSpecifiedTask( int task ) 
{
	if ( task < 0 || task >= nTasks ) {
		// If task is out of range, show no task selected.
		taskList->SelectedIndex = -1;
		taskIDBox->Text = "";
		// Clear the step list to reflect that there is no task selected.
		stepList->Items->Clear();
		nSteps = 0;
	}
	else {
		// Load the steps in the selected task.
		ParseTaskFile( task_file_list[task] );
		// Show which task is selected in the ListBox and TextBox of the GUI.
		taskList->SelectedIndex = task;
		taskIDBox->Text = Convert::ToString( taskID[task] );
	}	
	// We don't know yet what protocol will be selected.
	GoToSpecifiedStep( UNDEFINED );
}

// Given an index into the current list of steps, update the message, picture and status accordingly 
//  and show the specified stap as the selected step in the stepList ListBox.
void GripMMIDesktop::GoToSpecifiedStep( int step ) 
{
	char local_message[1024], local_picture[1024];

	if ( step <= 0 ) {
		// If step is out of range, show no step selected.
		stepIDBox->Text = "";
		stepList->SelectedIndex = -1;
		// Clear the DEX display;
		strcpy( local_message, "" );
		strcpy( local_picture, "blank.bmp" );
		fullStepForm->fullStep->Text = "";
		messageTypeBox->Text =	"";
	}
	else {
		// Center the selected line in the box.
		// This allows us to see what is going to come next.
		stepList->SelectedIndex = step;
		stepIDBox->Text = Convert::ToString( stepID[step] );

		// Show the full line in a larger, popup text box.
		// This box is typically hidden but can be shown by right-clicking on the step menu.
		fullStepForm->fullStep->Text = stepList->SelectedItem->ToString();

		// Update the representation of the GRIP display.
		// There are 'fake' buttons that show which options the user sees on the GRIP touchscreen.
		// Show or hide buttons according to what kind of text and message is current (status, query, alert).
		// Then show the picture and message that the subject sees at this step in the script.
		if ( type[step] == type_alert ) {
			fakeOK->Visible = false;
			fakeRetry->Visible = true;
			fakeIgnore->Visible = true;
			fakeCancel->Visible = true;
			fakeInterrupt->Visible = false;
			// Alerts are generated on GRIP when a test condition fails.
			// We cannot tell just by looking at the step ID whether the alert has
			// been triggered or if the test is running but the result is pending.
			// Housekeeping telemetry packets include a flag to say if the GRIP interpreter 
			// is in an error state or not and the packet parser will update the scriptErrorCheckbox accordingly.
			// So here we decide which message to show depending on whether the script crawler display is 'live', 
			// meaning it is following the step info sent in the housekeeping telemetry, and whether GRIP is in 
			// an error state or not.
			if ( scriptLiveCheckbox->Checked && scriptErrorCheckbox->Checked ) {
				// Error state means that the alert has been triggered.
				strcpy( local_message, message[step] );
				strcpy( local_picture, picture[step] );
				messageTypeBox->Text = "Alert (triggered)";
			}
			else if ( scriptLiveCheckbox->Checked ) {
				// Live, but no error state means that the test is still pending.
				strcpy( local_message, "Test pending ..." );
				strcpy( local_picture, "blank.bmp" );
				messageTypeBox->Text = "Alert (not triggered)";
			}
			else {
				// Not 'live' means that we are navigating through the script manually.
				// So here we show what could happen when the subject gets to this point in the script.
				strcpy( local_message, message[step] );
				strcpy( local_picture, picture[step] );
				messageTypeBox->Text = "Alert (shown if error)";
			}
		}
		else if ( type[step] == type_query ) {
			// Show the options available to the subject when queried.
			fakeOK->Visible = true;
			fakeRetry->Visible = false;
			fakeIgnore->Visible = false;
			fakeCancel->Visible = true;
			fakeInterrupt->Visible = false;
			// The text and picture of the query.
			strcpy( local_message, message[step] );
			strcpy( local_picture, picture[step] );
			// Inidicate that GRIP is waiting for the subject to respond.
			messageTypeBox->Text = "Query (Awaiting user response)";
		}
		else {
			fakeOK->Visible = false;
			fakeRetry->Visible = false;
			fakeIgnore->Visible = false;
			fakeCancel->Visible = false;
			fakeInterrupt->Visible = true;
			strcpy( local_message, message[step] );
			strcpy( local_picture, picture[step] );
			messageTypeBox->Text = "Status (Script will continue)";
		}
	}

	// Show the text message.
	// Convert and \n escape sequences within the message to newline. 
	for ( char *ptr = local_message; *ptr; ptr++ ) {
		if ( *ptr == '\\' && *(ptr+1) == 'n' ) {
			*ptr = '\r';
			*(ptr+1) = '\n';
		}
	}
	dexText->Text = gcnew String( local_message );

	// Show the picture.
	if ( strlen( local_picture ) ) {
		char picture_path[1024];
		strcpy( picture_path, pictureFilenamePrefix );
		strcat( picture_path, local_picture );
		dexPicture->ImageLocation = gcnew String( picture_path );
		dexPicture->Load();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////

// Position the script selections according to the specified user, protocol, task and step IDs.
// Note that unlike the routines above, the subject, protocol, task and step are specified as
//  their ID labels, not as the index into the respective tables, so we have to search for the
//  corresponding table entries and handle the case where we do not find them.

// This is used primarily when the the subject, protocol, task and step IDs are specified either 
//  by the operator using the editable text boxes on the GUI or when the housekeeping telemetry data
//  specifies new valuse.

void GripMMIDesktop::GoToSpecifiedIDs( int subject_id, int protocol_id, int task_id, int step_id ) 
{
	int i, current_selection;

	static bool guess_next = false;

	// Which subject is already selected?
	current_selection = subjectList->SelectedIndex;
	// Look for the desired subject ID in the table of available subject IDs.
	for ( i = 0; i < nSubjects - 1; i++ ) {
		if ( subjectID[i] == subject_id ) break;
	}
	// If we did not find the desired subject ID ...
	if ( subjectID[i] != subject_id ) GoToSpecifiedSubject( UNDEFINED );
	// If we found the desired ID and it is not already selected, change the selection.
	else if ( i != current_selection ) GoToSpecifiedSubject( i );

	// Which protocol is already selected?
	current_selection = protocolList->SelectedIndex;
	// Look for the desired protocol ID in the table of available protocols.
	for ( i = 0; i < nProtocols - 1; i++ ) {
		if ( protocolID[i] == protocol_id ) break;
	}
	// If we do not find the desired protocol ...
	if ( protocolID[i] != protocol_id ) GoToSpecifiedProtocol( UNDEFINED );
	// If we found the desired and it is not already selected, change the selection.
	else if ( i != current_selection ) GoToSpecifiedProtocol( i );

	// Which task is already selected?
	current_selection = taskList->SelectedIndex;

	// If both task_id and step_id go to zero, it means that the subjec just
	//  terminated a task. Guess that the next task in the list will be selected autormatically,
	//  even though the HK packet is saying that no task is selected.
	if ( task_id == 0 && step_id == 0 && guess_next ) {
		GoToSpecifiedTask( current_selection + 1 );
		// Do this only when task_id and step_id first go to zero.
		guess_next = false;
	}
	// If task_id and step_id are zero, but not for the first time, do nothing.
	else if ( task_id == 0 && step_id == 0 ) {}
	else {
		// Look for the desired task ID in the table of available tasks.
		for ( i = 0; i < nTasks; i++ ) {
			if ( taskID[i] == task_id ) break;
		}
		// If we do not find the desired task ...
		if ( taskID[i] != task_id ) GoToSpecifiedTask( UNDEFINED );
		// If we found the desired and it is not already selected, change the selection.
		else if ( i != current_selection ) {
			GoToSpecifiedTask( i );
			// Be ready if the task_id goes back to zero.
			guess_next = true;
		}
	}

	// Find the desired step number and go to it. Or to the nearest possible.
	for ( i = 0; i < nSteps; i++ ) {
		if ( stepID[i] >= step_id ) break;
	}
	GoToSpecifiedStep( i );

}

