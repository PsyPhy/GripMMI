#pragma once
#include "..\Useful\fOutputDebugString.h"
#include "..\PsyPhy2dGraphicsLib\Displays.h"
#include "..\PsyPhy2dGraphicsLib\Views.h"
#include "..\PsyPhy2dGraphicsLib\Layouts.h"
#include "..\Grip\DexAnalogMixin.h"

// Max number of frames (data slices)
#define MAX_FRAMES (12*60*60*20)
#define CODA_MARKERS 20
#define	CODA_UNITS	2
#define MANIPULANDUM_FIRST_MARKER 0
#define MANIPULANDUM_LAST_MARKER  7

#define PHASEPLOTS 3
#define STRIPCHARTS	6
#define SPAN_VALUES	6

extern Vector3 ManipulandumRotations[MAX_FRAMES];
extern Vector3 ManipulandumPosition[MAX_FRAMES];
extern float Acceleration[MAX_FRAMES][3];
extern float GripForce[MAX_FRAMES];
extern Vector3 LoadForce[MAX_FRAMES];
extern float NormalForce[N_FORCE_TRANSDUCERS][MAX_FRAMES];
extern double LoadForceMagnitude[MAX_FRAMES];
extern Vector3 CenterOfPressure[N_FORCE_TRANSDUCERS][MAX_FRAMES];
extern float RealMarkerTime[MAX_FRAMES];
extern float CompressedMarkerTime[MAX_FRAMES];
extern float RealAnalogTime[MAX_FRAMES];
extern float CompressedAnalogTime[MAX_FRAMES];
extern char  MarkerVisibility[MAX_FRAMES][CODA_MARKERS];
extern char  ManipulandumVisibility[MAX_FRAMES];
extern char markerVisibilityString[CODA_UNITS][32];
extern unsigned int nFrames;
extern int windowSpan[SPAN_VALUES];

namespace GripMMI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Timers;

	/// <summary>
	/// The single window that represents the GripMMI workspace.
	/// </summary>
	public ref class GripMMIDesktop : public System::Windows::Forms::Form
	{
	public:
		GripMMIDesktop(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
			InitializeGraphics();
			SimulateGripRT();
			AdjustSliders();
			CreateTimer();
			StartTimer();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~GripMMIDesktop()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::Button^  button1;

	private: System::Windows::Forms::GroupBox^  groupBox1;
	private: System::Windows::Forms::GroupBox^  groupBox2;
	private: System::Windows::Forms::GroupBox^  groupBox3;
	private: System::Windows::Forms::GroupBox^  groupBox4;
	private: System::Windows::Forms::GroupBox^  groupBox5;

	private: System::Windows::Forms::PictureBox^  LogoPictureBox;
	private: System::Windows::Forms::PictureBox^  XYPlot;
	private: System::Windows::Forms::PictureBox^  CoPPlot;
	private: System::Windows::Forms::PictureBox^  ZYPlot;
	private: System::Windows::Forms::PictureBox^  StripCharts;

	private: System::Windows::Forms::CheckBox^  FilterCheckbox;
	private: System::Windows::Forms::CheckBox^  DataLiveCheckbox;
	private: System::Windows::Forms::HScrollBar^  scrollBar;
	private: System::Windows::Forms::TrackBar^  spanSelector;

	private: 
		static System::Timers::Timer^ timer;
		void CreateTimer( void ) {
			timer = gcnew System::Timers::Timer( 5000 );
			timer->AutoReset = false;
			timer->Elapsed += gcnew System::Timers::ElapsedEventHandler( this, &GripMMI::GripMMIDesktop::OnTimerElapsed );
		}
		void StartTimer( void ) {
			timer->Enabled = true;
		}
		void OnTimerElapsed( System::Object^ source, System::Timers::ElapsedEventArgs^ e ) {
			fOutputDebugString( "Timer triggered.\n" );
			// SimulateGripRT();
			RefreshGraphics();
			StartTimer();
		}

	protected: 

	private: 

		::Display xy_display;
		::Display zy_display;
		::Display cop_display;
		::Display stripchart_display;
		::Layout  stripchart_layout;
		::View	visibility_view;
		::View  xy_view;
		::View  zy_view;
		::View  cop_view;
		System::Void InitializeGraphics( void );
		System::Void RefreshGraphics( void );
		System::Void KillGraphics( void );
		System::Void ChangeGraphics( void );
		System::Void AdjustSliders( void );

		// GripMMIGraphics.cpp
		void ResetBuffers( void );
		void GraphManipulandumPosition( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame );
		void GraphManipulandumRotations( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame );
		void PlotManipulandumPosition( double start_instant, double stop_instant, int start_frame, int stop_frame );
		void GraphLoadForce( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) ;
		void GraphAcceleration( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) ;
		void GraphGripForce( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) ;
		void GraphVisibility( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame ) ;
		void GraphCoP( ::View view, double start_instant, double stop_instant, int start_frame, int stop_frame );
		void PlotCoP( double start_window, double stop_window, int start_frame, int stop_frame );

		// GripMMIData.cpp
		void SimulateGripRT ( void );

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(GripMMIDesktop::typeid));
			this->LogoPictureBox = (gcnew System::Windows::Forms::PictureBox());
			this->XYPlot = (gcnew System::Windows::Forms::PictureBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->ZYPlot = (gcnew System::Windows::Forms::PictureBox());
			this->groupBox3 = (gcnew System::Windows::Forms::GroupBox());
			this->CoPPlot = (gcnew System::Windows::Forms::PictureBox());
			this->groupBox4 = (gcnew System::Windows::Forms::GroupBox());
			this->StripCharts = (gcnew System::Windows::Forms::PictureBox());
			this->groupBox5 = (gcnew System::Windows::Forms::GroupBox());
			this->scrollBar = (gcnew System::Windows::Forms::HScrollBar());
			this->spanSelector = (gcnew System::Windows::Forms::TrackBar());
			this->FilterCheckbox = (gcnew System::Windows::Forms::CheckBox());
			this->DataLiveCheckbox = (gcnew System::Windows::Forms::CheckBox());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LogoPictureBox))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->XYPlot))->BeginInit();
			this->groupBox1->SuspendLayout();
			this->groupBox2->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ZYPlot))->BeginInit();
			this->groupBox3->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->CoPPlot))->BeginInit();
			this->groupBox4->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->StripCharts))->BeginInit();
			this->groupBox5->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->spanSelector))->BeginInit();
			this->SuspendLayout();
			// 
			// LogoPictureBox
			// 
			this->LogoPictureBox->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"LogoPictureBox.Image")));
			this->LogoPictureBox->Location = System::Drawing::Point(24, 17);
			this->LogoPictureBox->Name = L"LogoPictureBox";
			this->LogoPictureBox->Size = System::Drawing::Size(230, 230);
			this->LogoPictureBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->LogoPictureBox->TabIndex = 0;
			this->LogoPictureBox->TabStop = false;
			// 
			// XYPlot
			// 
			this->XYPlot->BackColor = System::Drawing::Color::Maroon;
			this->XYPlot->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->XYPlot->Location = System::Drawing::Point(18, 25);
			this->XYPlot->Name = L"XYPlot";
			this->XYPlot->Size = System::Drawing::Size(246, 198);
			this->XYPlot->TabIndex = 2;
			this->XYPlot->TabStop = false;
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(1115, 17);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(123, 51);
			this->button1->TabIndex = 3;
			this->button1->Text = L"button1";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &GripMMIDesktop::button1_Click);
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->XYPlot);
			this->groupBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox1->Location = System::Drawing::Point(260, 12);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(279, 235);
			this->groupBox1->TabIndex = 4;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Frontal (XY)";
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->ZYPlot);
			this->groupBox2->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox2->Location = System::Drawing::Point(545, 12);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(279, 235);
			this->groupBox2->TabIndex = 5;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"Sagittal (ZY)";
			// 
			// ZYPlot
			// 
			this->ZYPlot->BackColor = System::Drawing::Color::Maroon;
			this->ZYPlot->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->ZYPlot->Location = System::Drawing::Point(17, 25);
			this->ZYPlot->Name = L"ZYPlot";
			this->ZYPlot->Size = System::Drawing::Size(246, 198);
			this->ZYPlot->TabIndex = 2;
			this->ZYPlot->TabStop = false;
			// 
			// groupBox3
			// 
			this->groupBox3->Controls->Add(this->CoPPlot);
			this->groupBox3->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox3->Location = System::Drawing::Point(830, 12);
			this->groupBox3->Name = L"groupBox3";
			this->groupBox3->Size = System::Drawing::Size(279, 235);
			this->groupBox3->TabIndex = 6;
			this->groupBox3->TabStop = false;
			this->groupBox3->Text = L"CoP";
			// 
			// CoPPlot
			// 
			this->CoPPlot->BackColor = System::Drawing::Color::Maroon;
			this->CoPPlot->BorderStyle = System::Windows::Forms::BorderStyle::FixedSingle;
			this->CoPPlot->Location = System::Drawing::Point(18, 25);
			this->CoPPlot->Name = L"CoPPlot";
			this->CoPPlot->Size = System::Drawing::Size(246, 198);
			this->CoPPlot->TabIndex = 2;
			this->CoPPlot->TabStop = false;
			// 
			// groupBox4
			// 
			this->groupBox4->Controls->Add(this->StripCharts);
			this->groupBox4->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox4->Location = System::Drawing::Point(24, 328);
			this->groupBox4->Name = L"groupBox4";
			this->groupBox4->Size = System::Drawing::Size(1087, 680);
			this->groupBox4->TabIndex = 7;
			this->groupBox4->TabStop = false;
			this->groupBox4->Text = L"Strip Charts";
			// 
			// StripCharts
			// 
			this->StripCharts->Location = System::Drawing::Point(6, 29);
			this->StripCharts->Name = L"StripCharts";
			this->StripCharts->Size = System::Drawing::Size(1075, 645);
			this->StripCharts->TabIndex = 0;
			this->StripCharts->TabStop = false;
			// 
			// groupBox5
			// 
			this->groupBox5->BackColor = System::Drawing::Color::Transparent;
			this->groupBox5->Controls->Add(this->scrollBar);
			this->groupBox5->Controls->Add(this->spanSelector);
			this->groupBox5->Controls->Add(this->FilterCheckbox);
			this->groupBox5->Controls->Add(this->DataLiveCheckbox);
			this->groupBox5->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox5->Location = System::Drawing::Point(24, 253);
			this->groupBox5->Name = L"groupBox5";
			this->groupBox5->Size = System::Drawing::Size(1087, 69);
			this->groupBox5->TabIndex = 8;
			this->groupBox5->TabStop = false;
			this->groupBox5->Text = L"Data Display";
			// 
			// scrollBar
			// 
			this->scrollBar->LargeChange = 100000;
			this->scrollBar->Location = System::Drawing::Point(238, 24);
			this->scrollBar->Maximum = 864000;
			this->scrollBar->Name = L"scrollBar";
			this->scrollBar->Size = System::Drawing::Size(767, 25);
			this->scrollBar->SmallChange = 10000;
			this->scrollBar->TabIndex = 10;
			this->scrollBar->Value = 1000;
			this->scrollBar->ValueChanged += gcnew System::EventHandler(this, &GripMMIDesktop::scrollBar_ValueChanged);
			// 
			// spanSelector
			// 
			this->spanSelector->BackColor = System::Drawing::Color::White;
			this->spanSelector->LargeChange = 1;
			this->spanSelector->Location = System::Drawing::Point(13, 19);
			this->spanSelector->Margin = System::Windows::Forms::Padding(1);
			this->spanSelector->Maximum = 5;
			this->spanSelector->Name = L"spanSelector";
			this->spanSelector->Size = System::Drawing::Size(219, 45);
			this->spanSelector->TabIndex = 11;
			this->spanSelector->TickStyle = System::Windows::Forms::TickStyle::TopLeft;
			this->spanSelector->ValueChanged += gcnew System::EventHandler(this, &GripMMIDesktop::spanSelector_ValueChanged);
			// 
			// FilterCheckbox
			// 
			this->FilterCheckbox->AutoSize = true;
			this->FilterCheckbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->FilterCheckbox->Location = System::Drawing::Point(1017, 35);
			this->FilterCheckbox->Name = L"FilterCheckbox";
			this->FilterCheckbox->Size = System::Drawing::Size(63, 24);
			this->FilterCheckbox->TabIndex = 9;
			this->FilterCheckbox->Text = L"Filter";
			this->FilterCheckbox->UseVisualStyleBackColor = true;
			// 
			// DataLiveCheckbox
			// 
			this->DataLiveCheckbox->AutoSize = true;
			this->DataLiveCheckbox->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 12, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->DataLiveCheckbox->Location = System::Drawing::Point(1017, 14);
			this->DataLiveCheckbox->Name = L"DataLiveCheckbox";
			this->DataLiveCheckbox->Size = System::Drawing::Size(56, 24);
			this->DataLiveCheckbox->TabIndex = 0;
			this->DataLiveCheckbox->Text = L"Live";
			this->DataLiveCheckbox->UseVisualStyleBackColor = true;
			// 
			// GripMMIDesktop
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::Color::White;
			this->ClientSize = System::Drawing::Size(1592, 1016);
			this->Controls->Add(this->groupBox5);
			this->Controls->Add(this->groupBox4);
			this->Controls->Add(this->groupBox3);
			this->Controls->Add(this->groupBox2);
			this->Controls->Add(this->groupBox1);
			this->Controls->Add(this->button1);
			this->Controls->Add(this->LogoPictureBox);
			this->MaximumSize = System::Drawing::Size(1600, 1200);
			this->Name = L"GripMMIDesktop";
			this->Text = L"GripMMI";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &GripMMIDesktop::GripMMIDesktop_FormClosing);
			this->Load += gcnew System::EventHandler(this, &GripMMIDesktop::GripMMIDesktop_Load);
			this->Paint += gcnew System::Windows::Forms::PaintEventHandler(this, &GripMMIDesktop::GripMMIDesktop_Paint);
			this->KeyPress += gcnew System::Windows::Forms::KeyPressEventHandler(this, &GripMMIDesktop::GripMMIDesktop_KeyPress);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LogoPictureBox))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->XYPlot))->EndInit();
			this->groupBox1->ResumeLayout(false);
			this->groupBox2->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->ZYPlot))->EndInit();
			this->groupBox3->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->CoPPlot))->EndInit();
			this->groupBox4->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->StripCharts))->EndInit();
			this->groupBox5->ResumeLayout(false);
			this->groupBox5->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->spanSelector))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion

	private: System::Void GripMMIDesktop_Paint(System::Object^  sender, System::Windows::Forms::PaintEventArgs^  e) {
		// The graphs may need to be refreshed.
		// RefreshGraphics();
	}

	private: System::Void GripMMIDesktop_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
		 // Free resources allocated by the PsyPhy graphics routines.
		 KillGraphics();
	}

	private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) {
			 SimulateGripRT ();
			 RefreshGraphics();
	}
private: System::Void GripMMIDesktop_Load(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void GripMMIDesktop_KeyPress(System::Object^  sender, System::Windows::Forms::KeyPressEventArgs^  e) {
		 }
private: System::Void spanSelector_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			 AdjustSliders();
			 RefreshGraphics();
		 }
private: System::Void scrollBar_ValueChanged(System::Object^  sender, System::EventArgs^  e) {
			 RefreshGraphics();
		 }

};

}

