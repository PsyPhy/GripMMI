#pragma once
///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/PsyPhy/GripMMI
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// Creates a dialog to show when the GripMMI is waiting for the first packets to arrive.

// Number of milliseconds to wait be attempts to access packet cache files.
#define RETRY_TIMEOUT	1000

namespace GripMMI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;


	/// <summary>
	/// Summary for GripMMIStartup
	/// </summary>
	public ref class GripMMIStartup : public System::Windows::Forms::Form
	{
	public:
		GripMMIStartup( const char *version_string )
		{
			InitializeComponent();
			versionText->Text = gcnew String( version_string );
			DisplayCachePaths();
			CreateRefreshTimer( RETRY_TIMEOUT );
			StartRefreshTimer();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~GripMMIStartup()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::PictureBox^	LogoPictureBox;
	private: System::Windows::Forms::Button^		startupCancelButton;
	private: System::Windows::Forms::Label^			versionText;

	private: System::Windows::Forms::Label^			label1;
	private: System::Windows::Forms::Label^  rtCacheFilenameText;
	private: System::Windows::Forms::Label^  hkCacheFilenameText;




	protected: 
		static Timer^ timer;
		void DisplayCachePaths( void );
		void CreateRefreshTimer( int interval );
		void StartRefreshTimer( void );
		void StopRefreshTimer( void );
		void OnTimerElapsed( System::Object^ source, System::EventArgs ^ e );


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
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(GripMMIStartup::typeid));
			this->LogoPictureBox = (gcnew System::Windows::Forms::PictureBox());
			this->startupCancelButton = (gcnew System::Windows::Forms::Button());
			this->versionText = (gcnew System::Windows::Forms::Label());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->rtCacheFilenameText = (gcnew System::Windows::Forms::Label());
			this->hkCacheFilenameText = (gcnew System::Windows::Forms::Label());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LogoPictureBox))->BeginInit();
			this->SuspendLayout();
			// 
			// LogoPictureBox
			// 
			this->LogoPictureBox->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"LogoPictureBox.Image")));
			this->LogoPictureBox->Location = System::Drawing::Point(2, 2);
			this->LogoPictureBox->Name = L"LogoPictureBox";
			this->LogoPictureBox->Size = System::Drawing::Size(144, 144);
			this->LogoPictureBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->LogoPictureBox->TabIndex = 8;
			this->LogoPictureBox->TabStop = false;
			// 
			// startupCancelButton
			// 
			this->startupCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->startupCancelButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->startupCancelButton->Location = System::Drawing::Point(296, 107);
			this->startupCancelButton->Name = L"startupCancelButton";
			this->startupCancelButton->Size = System::Drawing::Size(72, 26);
			this->startupCancelButton->TabIndex = 7;
			this->startupCancelButton->Text = L"Cancel";
			this->startupCancelButton->UseVisualStyleBackColor = true;
			this->startupCancelButton->Click += gcnew System::EventHandler(this, &GripMMIStartup::startupCancelButton_Click);
			// 
			// versionText
			// 
			this->versionText->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->versionText->Location = System::Drawing::Point(262, 15);
			this->versionText->Name = L"versionText";
			this->versionText->Size = System::Drawing::Size(140, 13);
			this->versionText->TabIndex = 9;
			this->versionText->Text = L"GripMMI V \?.\?";
			this->versionText->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(152, 41);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(114, 13);
			this->label1->TabIndex = 11;
			this->label1->Text = L"Waiting for packets at:";
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// rtCacheFilenameText
			// 
			this->rtCacheFilenameText->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->rtCacheFilenameText->Location = System::Drawing::Point(169, 54);
			this->rtCacheFilenameText->Name = L"rtCacheFilenameText";
			this->rtCacheFilenameText->Size = System::Drawing::Size(327, 16);
			this->rtCacheFilenameText->TabIndex = 12;
			this->rtCacheFilenameText->Text = L"reatime cache";
			this->rtCacheFilenameText->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// hkCacheFilenameText
			// 
			this->hkCacheFilenameText->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Regular, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->hkCacheFilenameText->Location = System::Drawing::Point(169, 70);
			this->hkCacheFilenameText->Name = L"hkCacheFilenameText";
			this->hkCacheFilenameText->Size = System::Drawing::Size(327, 15);
			this->hkCacheFilenameText->TabIndex = 13;
			this->hkCacheFilenameText->Text = L"housekeeping cache";
			this->hkCacheFilenameText->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// GripMMIStartup
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::Window;
			this->ClientSize = System::Drawing::Size(509, 154);
			this->Controls->Add(this->hkCacheFilenameText);
			this->Controls->Add(this->rtCacheFilenameText);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->versionText);
			this->Controls->Add(this->LogoPictureBox);
			this->Controls->Add(this->startupCancelButton);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"GripMMIStartup";
			this->Text = L"GripMMI Startup";
			this->Load += gcnew System::EventHandler(this, &GripMMIStartup::GripMMIStartup_Load);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LogoPictureBox))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


private: System::Void startupCancelButton_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void pictureBox1_Click(System::Object^  sender, System::EventArgs^  e) {
		 }
private: System::Void GripMMIStartup_Load(System::Object^  sender, System::EventArgs^  e) {
		 }
};
}
