// GripMMIScripts.cpp : methods for drawing the various graphs on the screen.

//#pragma warning disable 4996

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

char packetBufferPathRoot[MAX_PATHLENGTH];
char scriptDirectory[MAX_PATHLENGTH];
char pictureFilenamePrefix[MAX_PATHLENGTH];

char session_file[MAX_MENU_ITEMS][MAX_MENU_ITEM_LENGTH];
char protocol_file[MAX_MENU_ITEMS][MAX_MENU_ITEM_LENGTH];
char task_file[MAX_MENU_ITEMS][MAX_MENU_ITEM_LENGTH];

char picture[MAX_STEPS][256];
char message[MAX_STEPS][132];
char *type[MAX_STEPS];
bool comment[MAX_STEPS];

int  stepID[MAX_STEPS];
int  taskID[MAX_STEPS];
int  protocolID[MAX_STEPS];
int  subjectID[MAX_STEPS];

char *type_status = "Status (script will continue)";
char *type_query =  "Query (waiting for response)";
char *type_alert =  "Alert (seen only if error)";

// A convenient macro to hand message/picture pairs.
#define add_to_message_pair(x,y,z) { strcpy( message[lines], y ); if ( z ) strcpy( picture[lines], z ); else strcpy( picture[lines], "" ); type[lines] = type_alert; }

enum { NORMAL_EXIT = 0, NO_USER_FILE, NO_LOG_FILE, ERROR_EXIT };

void GripMMIDesktop::ParseTaskFile ( const char *filename ) {

	FILE *fp;

	int tokens;
	char *token[MAX_TOKENS];
	char line[2048];
	int line_n = 0;

	int  lines = 0;
	int  current_step = 0;

	char msg[1024];

	char status_picture[256] = "blank.bmp";
	char status_message[256] = "";

	fp = fopen( filename, "r" );
	if ( !fp ) {
		// Signal the error.
		sprintf( msg, "Error opening task file %s for read.\n", filename );
		::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
		return;
	}

	// SendDlgItemMessage( IDC_STEPS, LB_RESETCONTENT, 0, 0 );
	stepList->Items->Clear();

	// SendDlgItemMessage( IDC_STEPS, LB_ADDSTRING, 0, (LPARAM) "<Waiting to start ...>" );
	stepList->Items->Add( "<Waiting to start ...>" );
	strcpy( message[lines], status_message );
	strcpy( picture[lines], status_picture );
	type[lines] = type_status;
	comment[lines] = true;
	stepID[lines] = current_step;
	lines++;

	while ( fgets( line, sizeof( line ), fp ) ) {

		line[strlen( line ) - 1] = 0;
		String^ line_string = gcnew String( line );
		// SendDlgItemMessage( IDC_STEPS, LB_ADDSTRING, 0, (LPARAM) line );
		stepList->Items->Add( line_string );
		tokens = ParseCommaDelimitedLine( token, line );

		if ( tokens ) {

			if ( !strcmp( token[0], "CMD_LOG_MESSAGE" ) ) {
				int value, items;
				if ( !strcmp( token[1], "logmsg" ) ) value = 0;
				else if ( !strcmp( token[1], "usermsg" ) ) value = 1;
				else {
					items = sscanf( token[1], "%d", &value );
					value = items && value;
				}
				if ( value != 0 ) {
					if ( token[2] ) strcpy( status_message, token[2] );
					else strcpy( status_message, "" );
				}
			}
			else if ( !strcmp( token[0], "CMD_SET_PICTURE" ) ) {
				if ( tokens > 1 ) strcpy( status_picture, token[1] );
				else strcpy( status_picture, "blank.bmp" );
			}


			if ( !strcmp( token[0], "CMD_WAIT_SUBJ_READY" ) ) {
				strcpy( message[lines], token[1] );
				strcpy( picture[lines], token[2] );
				type[lines] = type_query;
			}

			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_ATTARGET" ) ) add_to_message_pair( &alert, token[13], token[14] )		
			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_GRIP" ) ) add_to_message_pair( &alert, token[4], token[5] )
			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_GRIPFORCE" ) ) add_to_message_pair( &alert, token[11], token[12] )
			else if ( !strcmp( token[0], "CMD_WAIT_MANIP_SLIP" ) ) add_to_message_pair( &alert, token[11], token[12] )
			else if ( !strcmp( token[0], "CMD_CHK_MASS_SELECTION" ) ) add_to_message_pair( &alert, "Put mass in cradle X and pick up mass from cradle Y.", "TakeMass.bmp" )
			else if ( !strcmp( token[0], "CMD_CHK_HW_CONFIG" ) ) add_to_message_pair( &alert, token[1], token[2] )
			else if ( !strcmp( token[0], "CMD_ALIGN_CODA" ) ) add_to_message_pair( &alert, token[1], token[2] )
			else if ( !strcmp( token[0], "CMD_CHK_CODA_ALIGNMENT" ) ) add_to_message_pair( &alert, token[4], token[5] )
			else if ( !strcmp( token[0], "CMD_CHK_CODA_FIELDOFVIEW" ) ) add_to_message_pair( &alert, token[9], token[10] )
			else if ( !strcmp( token[0], "CMD_CHK_CODA_PLACEMENT" ) ) add_to_message_pair( &alert, token[11], token[12] )
			else if ( !strcmp( token[0], "CMD_CHK_MOVEMENTS_AMPL" ) ) add_to_message_pair( &alert, token[6], token[7] )
			else if ( !strcmp( token[0], "CMD_CHK_MOVEMENTS_CYCLES" ) ) add_to_message_pair( &alert, token[7], token[8] )
			else if ( !strcmp( token[0], "CMD_CHK_START_POS" ) ) add_to_message_pair( &alert, token[7], token[8] )
			else if ( !strcmp( token[0], "CMD_CHK_MOVEMENTS_DIR" ) ) add_to_message_pair( &alert, token[6], token[7] )
			else if ( !strcmp( token[0], "CMD_CHK_COLLISIONFORCE" ) ) add_to_message_pair( &alert, token[4], token[5] )
			else if ( !strcmp( token[0], "CMD_CHK_MANIP_VISIBILITY" ) ) add_to_message_pair( &alert, token[3], token[4] )

			else {
				strcpy( message[lines], status_message );
				strcpy( picture[lines], status_picture );
				type[lines] = type_status;
			}
			comment[lines] = false;
			current_step++;
			stepID[lines] = current_step;
		}
		else {
			strcpy( message[lines], status_message );
			strcpy( picture[lines], status_picture );
			type[lines] = type_status;
			comment[lines] = true;
			stepID[lines] = current_step;
		}
		
		lines++;

	}
	
	fclose( fp );

	// nSteps = current_step;
	nSteps = lines;

	// SendDlgItemMessage( IDC_STEPS, LB_ADDSTRING, 0, (LPARAM) "************ End of Script ************" );
	stepList->Items->Add( "************ End of Script ************" );
	strcpy( message[lines], "*********** End of Script ***********\n*********** End of Script ***********\n*********** End of Script ***********\n*********** End of Script ***********" );
	strcpy( picture[lines], "blank.bmp" );
	type[lines] = type_status;
	comment[lines] = true;
	stepID[lines] = 9999;



}

void GripMMIDesktop::ParseProtocolFile ( const char *filename ) {

	FILE *fp;

	int tokens;
	char *token[MAX_TOKENS];
	char line[2048];
	int line_n = 0;

	char task_filename[1024];
	char msg[1024];
	int i;

	nTasks = 0;

	// We want to work in the same directory as the subject file.
	// Get the directory from the filename.
	char directory[1024];
	for ( i = strlen( filename ); i >=0; i-- ) {
		if ( filename[i] == '\\' ) break;
	}
	strncpy( directory, filename, i + 1 );
	directory[i+1] = 0;


	fp = fopen( filename, "r" );
	if ( !fp ) {
		// Signal the error.
		sprintf( msg, "Error opening protocol file %s for read.\n", filename );
		::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
		return;
	}

	// Work our way through the protocol file line-by-line.
	//SendDlgItemMessage( IDC_TASKS, LB_RESETCONTENT, 0, 0 );
	taskList->Items->Clear();

	while ( fgets( line, sizeof( line ), fp ) ) {

		line_n++;
		tokens = ParseCommaDelimitedLine( token, line );

		// Lines in protocol files are divided into 4 fields.
		if ( tokens == 4 ) {
			// First parameter must be CMD_TASK.
			if ( strcmp( token[0], "CMD_TASK" ) ) {
				sprintf( msg, "%s Line %03d Command not CMD_TASK: %s\n", filename, line_n, token[0] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
			}
			// Second parameter is a task ID. Should be an integer value.
			if ( 1 != sscanf( token[1], "%d", &taskID[nTasks] ) ) {
				sprintf( msg, "%s Line %03d Error reading task ID: %s\n", filename, line_n, token[1] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				exit( - 1 );
			}
			// The third item is the name of the protocol file.
			// Check if it is present and readable, unless told to ignore it.
			if ( !strstr( token[2], "ignore" ) ) {
				strncpy( task_filename, directory, sizeof( task_filename ) );
				strncat( task_filename, token[2],  sizeof( task_filename ) );
				if ( _access( task_filename, 0x00 ) ) {
					sprintf( msg, "%s Line %03d Cannot access protocol file: %s\n", filename, line_n, task_filename );
					::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				}
				else {
					// Add the filename to the list of protocols.
					strcpy( task_file[nTasks], task_filename );
					// SendDlgItemMessage( IDC_TASKS, LB_ADDSTRING, 0, (LPARAM) token[3] );
					taskList->Items->Add( gcnew String( token[3] ) );
					nTasks++;
				}
			}

		}
		else if ( tokens != 0 ) {
			sprintf( msg, "%s Line %03d Wrong number of parameters: %s\n", filename, line_n, line );
			::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
			return;
		}

	}

	fclose( fp );

}

void GripMMIDesktop::ParseSessionFile ( const char *filename ) {

	FILE *fp;

	int tokens;
	char *token[MAX_TOKENS];
	char line[2048];
	int line_n = 0;

	char protocol_filename[1024];
	char msg[1024];
	int	i;

	nProtocols = 0;

	// We want to work in the same directory as the subject file.
	// Get the directory from the filename.
	char directory[1024];
	for ( i = strlen( filename ); i >=0; i-- ) {
		if ( filename[i] == '\\' ) break;
	}
	strncpy( directory, filename, i + 1 );
	directory[i+1] = 0;


	// Open the session file, if we can.
	fp = fopen( filename, "r" );
	if ( !fp ) {
		// Signal the error.
		sprintf( msg, "Error opening session file %s for read.\n", filename );
		::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
		return;
	}

	// Work our way through the session file line-by-line.
	// SendDlgItemMessage( IDC_PROTOCOLS, LB_RESETCONTENT, 0, 0 );
	protocolList->Items->Clear();
	while ( fgets( line, sizeof( line ), fp ) ) {

		line_n++;
		tokens = ParseCommaDelimitedLine( token, line );

		// Lines in session files are divided into 4 fields.
		if ( tokens == 4 ) {
			// First parameter must be CMD_PROTOCOL.
			if ( strcmp( token[0], "CMD_PROTOCOL" ) ) {
				sprintf( msg, "%s Line %03d Command not CMD_PROTOCOL: %s\n", filename, line_n, token[0] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
			}
			// Second parameter is a protocol ID. Should be an integer value.
			if ( 1 != sscanf( token[1], "%d", &protocolID[nProtocols] ) ) {
				sprintf( msg, "%s Line %03d Error reading protocol ID: %s\n", filename, line_n, token[1] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				exit( - 1 );
			}
			// The third item is the name of the protocol file.
			// Check if it is present and readable, unless told to ignore it.
			if ( !strstr( token[2], "ignore" ) ) {
				strncpy( protocol_filename, directory, sizeof( protocol_filename ) );
				strncat( protocol_filename, token[2], sizeof( protocol_filename ) );
				if ( _access( protocol_filename, 0x00 ) ) {
					sprintf( msg, "%s Line %03d Cannot access protocol file: %s\n", filename, line_n, protocol_filename );
					::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				}
				else {
					// Add the filename to the list of protocols.
					strcpy( protocol_file[nProtocols], protocol_filename );
					// SendDlgItemMessage( IDC_PROTOCOLS, LB_ADDSTRING, 0, (LPARAM) token[3] );
					protocolList->Items->Add( gcnew String( token[3] ) );
					nProtocols++;
				}
			}

		}
		else if ( tokens != 0 ) {
			sprintf( msg, "%s Line %03d Wrong number of parameters: %s\n", filename, line_n, line );
			::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
			return;
		}

	}

	fclose( fp );

}


int GripMMIDesktop::ParseSubjectFile ( const char *filename ) {

	FILE *fp;

	int tokens;
	char *token[MAX_TOKENS];
	char line[2048];
	int line_n = 0;
	int i;

	char session_filename[1024];

	nSubjects = 0;

	// We want to work in the same directory as the subject file.
	// Get the directory from the filename.
	char directory[1024];
	for ( i = strlen( filename ); i >=0; i-- ) {
		if ( filename[i] == '\\' ) break;
	}
	strncpy( directory, filename, i + 1 );
	directory[i+1] = 0;

	// Open the root file, if we can.
	fp = fopen( filename, "r" );
	if ( !fp ) {
		char msg[1024];
		sprintf( msg, "Error opening subject file %s for read.", filename );
		printf( "%s\n", msg );
		::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
		return( ERROR_EXIT );
	}

	// Step through line by line and follow the links to the session files.
	//	SendDlgItemMessage( IDC_SUBJECTS, LB_RESETCONTENT, 0, 0 );
	subjectList->Items->Clear();
	while ( fgets( line, sizeof( line ), fp ) ) {

		// Count the lines.
		line_n++;

		// Break the line into pieces as defined by the DEX/GRIP ICD.
		tokens = ParseCommaDelimitedLine( token, line );

		// Parse each line and do some syntax error checking.
		if ( tokens == 5 ) {

			// First parameter must be CMD_USER.
			if ( strcmp( token[0], "CMD_USER" ) ) {
				char msg[1024];
				sprintf( msg, "Line %03d Command not CMD_USER: %s\n", line_n, token[0] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				exit( ERROR_EXIT );
			}
			// Second parameter is a subject ID. Should be an integer value.
			// Verify also that subject IDs are unique.
			if ( 1 != sscanf( token[1], "%d", &subjectID[nSubjects] ) ) {
				char msg[1024];
				// Report error for invalid subject ID field.
				sprintf( msg, "Line %03d Error reading subject ID: %s\n", line_n, token[1] );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				exit( ERROR_EXIT );
			}
			// The third parameter is the name of the session file.
			// Here we check that it is present and if so, we process it as well.
			strncpy( session_filename, directory, sizeof( session_filename ) );
			strncat( session_filename, token[3], sizeof( session_filename ) );
			if ( _access( session_filename, 0x00 ) ) {
				char msg[1024];
				// The file must not only be present, it also has to be readable.
				// I had a funny problem with this at one point. Maybe MAC OS had created links.
				sprintf( msg, "Line %03d Cannot access session file: %s\n", line_n, session_filename );
				::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
				exit( ERROR_EXIT );
			}	
			else {
			
				strcpy( session_file[nSubjects], session_filename );
				// SendDlgItemMessage( IDC_SUBJECTS, LB_ADDSTRING, 0, (LPARAM) token[4] );
				subjectList->Items->Add( gcnew String( token[4] ));
				nSubjects++;
			
			}
			// There is a fifth field to the line, which is text describing the subject.
			// For the moment I don't do any error checking on that parameter.
		}
		else if ( tokens != 0 ) {
			char msg[1024];
			sprintf( msg, "%s Line %03d Wrong number of parameters: %s\n", filename, line_n, line );
			::MessageBox( NULL, msg, "DexScriptRunner", MB_OK | MB_ICONERROR );
			exit( ERROR_EXIT );
		}			
	}

	fclose( fp );
	return( 0 );

}

void GripMMIDesktop::GoToSpecifiedSubject( int subject ) 
{
	if ( subject < 0 || subject >= nSubjects ) {
		// Show no subject selected.
		// SendDlgItemMessage( IDC_SUBJECTS, LB_SETCURSEL, -1, 0 );
		subjectList->SelectedIndex = -1;
		// SetDlgItemInt( IDC_SUBJECTID, 0 );
		subjectIDBox->Text = "";
		// Clear the protocol list to reflect that there is no subject selected.
		// SendDlgItemMessage( IDC_PROTOCOLS, LB_RESETCONTENT, 0, 0 );
		protocolList->Items->Clear();
		nProtocols = 0;
	}
	else {
		ParseSessionFile( session_file[subject] );
		// SendDlgItemMessage( IDC_SUBJECTS, LB_SETCURSEL, subject, 0 );
		subjectList->SelectedIndex = subject;
		// SetDlgItemInt( IDC_SUBJECTID, subjectID[subject] );
		subjectIDBox->Text = Convert::ToString( subjectID[subject] );
	}	
	// We don't know yet what protocol will be selected.
	GoToSpecifiedProtocol( UNDEFINED );
}

void GripMMIDesktop::GoToSpecifiedProtocol( int protocol ) 
{
	if ( protocol < 0 || protocol >= nProtocols ) {
		// Show no protocol selected.
		protocolList->SelectedIndex = -1;
		protocolIDBox->Text = "";
		// Clear the protocol list to reflect that there is no subject selected.
		taskList->Items->Clear();
		nTasks = 0;
	}
	else {
		ParseProtocolFile( protocol_file[protocol] );
		protocolList->SelectedIndex = protocol;
		protocolIDBox->Text = Convert::ToString( protocolID[protocol] );
	}	
	// We don't know yet what protocol will be selected.
	GoToSpecifiedTask( UNDEFINED );
}
void GripMMIDesktop::GoToSpecifiedTask( int task ) 
{
	if ( task < 0 || task >= nTasks ) {
		// Show no protocol selected.
		taskList->SelectedIndex = -1;
		taskIDBox->Text = "";
		// Clear the protocol list to reflect that there is no subject selected.
		stepList->Items->Clear();
		nSteps = 0;
	}
	else {
		ParseTaskFile( task_file[task] );
		taskList->SelectedIndex = task;
		taskIDBox->Text = Convert::ToString( taskID[task] );
	}	
	// We don't know yet what protocol will be selected.
	GoToSpecifiedStep( UNDEFINED );
}
void GripMMIDesktop::GoToSpecifiedStep( int step ) 
{
	char local_message[1024], local_picture[1024];

	if ( step <= 0 ) {
		// SetDlgItemInt( IDC_STEPID, 0 );
		stepIDBox->Text = "";
		stepList->SelectedIndex = -1;
		// Clear the DEX display;
		strcpy( local_message, "" );
		strcpy( local_picture, "blank.bmp" );
		fullStep->Text = "";
		messageTypeBox->Text =	"";
	}
	else {
		// Center the selected line in the box.
		// stepList->SelectedIndex = step + 14;
		// Select the specified step.
		stepList->SelectedIndex = step;
		stepIDBox->Text = Convert::ToString( stepID[step] );

		// Show the full line in a larger box.
		fullStep->Text = stepList->SelectedItem->ToString();
		if ( type[step] == type_alert ) {
			// Show or hide buttons accordingly.
			fakeOK->Visible = false;
			fakeRetry->Visible = true;
			fakeIgnore->Visible = true;
			fakeCancel->Visible = true;
			fakeInterrupt->Visible = false;
			if ( scriptLiveCheckbox->Checked && scriptErrorCheckbox->Checked ) {
				strcpy( local_message, message[step] );
				strcpy( local_picture, picture[step] );
				messageTypeBox->Text = "Alert (triggered)";
			}
			else if ( scriptLiveCheckbox->Checked ) {
				strcpy( local_message, "Test pending ..." );
				strcpy( local_picture, "blank.bmp" );
				messageTypeBox->Text = "Alert (not triggered)";
			}
			else {
				strcpy( local_message, message[step] );
				strcpy( local_picture, picture[step] );
				messageTypeBox->Text = "Alert (shown if error)";
			}
		}
		else if ( type[step] == type_query ) {
			fakeOK->Visible = true;
			fakeRetry->Visible = false;
			fakeIgnore->Visible = false;
			fakeCancel->Visible = true;
			fakeInterrupt->Visible = false;
			strcpy( local_message, message[step] );
			strcpy( local_picture, picture[step] );
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
		// Convert \n escape sequence to newline. 
		for ( char *ptr = local_message; *ptr; ptr++ ) {
			if ( *ptr == '\\' && *(ptr+1) == 'n' ) {
				*ptr = '\r';
				*(ptr+1) = '\n';
			}
		}
	}

	// Show the text message.
	dexText->Text = gcnew String( local_message );

	// Show the picture.
	if ( strlen( local_picture ) ) {
		char picture_path[1024];
		strncpy( picture_path, pictureFilenamePrefix, sizeof( picture_path ) );
		strncat( picture_path, local_picture, sizeof( picture_path ) );
		dexPicture->ImageLocation = gcnew String( picture_path );
		dexPicture->Load();
	}
}

// Position the script selections according to the specified user, protocol, task and step IDs.
// Note that unlike the routines above, the subject, protocol, task and step are specified as
//  their ID labels, not as the index into the respective tables, so we have to search for the
//  corresponding table entries and handle the case where we do not find them.
void GripMMIDesktop::GoToSpecifiedIDs( int subject_id, int protocol_id, int task_id, int step_id ) 
{
	int i, current_selection;

	static bool guess_next = false;

	// Which subject is already selected?
	// current_selection =  SendDlgItemMessage( IDC_SUBJECTS, LB_GETCURSEL, 0, 0 );
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
	//current_selection =  SendDlgItemMessage( IDC_PROTOCOLS, LB_GETCURSEL, 0, 0 );
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
	// current_selection =  SendDlgItemMessage( IDC_TASKS, LB_GETCURSEL, 0, 0 );
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

