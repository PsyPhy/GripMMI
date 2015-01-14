#pragma once

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
	private: System::Windows::Forms::PictureBox^	pictureBox1;
	private: System::Windows::Forms::Label^			label1;

	protected: 
		static Timer^ timer;
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
			this->pictureBox1 = (gcnew System::Windows::Forms::PictureBox());
			this->label1 = (gcnew System::Windows::Forms::Label());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LogoPictureBox))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->BeginInit();
			this->SuspendLayout();
			// 
			// LogoPictureBox
			// 
			this->LogoPictureBox->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"LogoPictureBox.Image")));
			this->LogoPictureBox->Location = System::Drawing::Point(2, 2);
			this->LogoPictureBox->Name = L"LogoPictureBox";
			this->LogoPictureBox->Size = System::Drawing::Size(132, 132);
			this->LogoPictureBox->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->LogoPictureBox->TabIndex = 8;
			this->LogoPictureBox->TabStop = false;
			// 
			// startupCancelButton
			// 
			this->startupCancelButton->DialogResult = System::Windows::Forms::DialogResult::Cancel;
			this->startupCancelButton->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, 
				System::Drawing::GraphicsUnit::Point, static_cast<System::Byte>(0)));
			this->startupCancelButton->Location = System::Drawing::Point(177, 108);
			this->startupCancelButton->Name = L"startupCancelButton";
			this->startupCancelButton->Size = System::Drawing::Size(72, 26);
			this->startupCancelButton->TabIndex = 7;
			this->startupCancelButton->Text = L"Cancel";
			this->startupCancelButton->UseVisualStyleBackColor = true;
			// 
			// versionText
			// 
			this->versionText->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 8.25F, System::Drawing::FontStyle::Bold, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->versionText->Location = System::Drawing::Point(140, 9);
			this->versionText->Name = L"versionText";
			this->versionText->Size = System::Drawing::Size(140, 13);
			this->versionText->TabIndex = 9;
			this->versionText->Text = L"GripMMI V \?.\?";
			this->versionText->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// pictureBox1
			// 
			this->pictureBox1->Image = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox1.Image")));
			this->pictureBox1->InitialImage = (cli::safe_cast<System::Drawing::Image^  >(resources->GetObject(L"pictureBox1.InitialImage")));
			this->pictureBox1->Location = System::Drawing::Point(177, 48);
			this->pictureBox1->Name = L"pictureBox1";
			this->pictureBox1->Size = System::Drawing::Size(71, 54);
			this->pictureBox1->SizeMode = System::Windows::Forms::PictureBoxSizeMode::StretchImage;
			this->pictureBox1->TabIndex = 10;
			this->pictureBox1->TabStop = false;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(152, 29);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(111, 13);
			this->label1->TabIndex = 11;
			this->label1->Text = L"Waiting for packets ...";
			this->label1->TextAlign = System::Drawing::ContentAlignment::MiddleCenter;
			// 
			// GripMMIStartup
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->BackColor = System::Drawing::SystemColors::Window;
			this->ClientSize = System::Drawing::Size(300, 138);
			this->Controls->Add(this->label1);
			this->Controls->Add(this->pictureBox1);
			this->Controls->Add(this->versionText);
			this->Controls->Add(this->LogoPictureBox);
			this->Controls->Add(this->startupCancelButton);
			this->MaximizeBox = false;
			this->MinimizeBox = false;
			this->Name = L"GripMMIStartup";
			this->Text = L"GripMMI Startup";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->LogoPictureBox))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->pictureBox1))->EndInit();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


};
}
