#pragma once

///
/// Module:	GripMMI
/// 
///	Author:					J. McIntyre, PsyPhy Consulting
/// Initial release:		18 December 2014
/// Modification History:	see https://github.com/frenchjam/GripGroundMonitorClient
///
/// Copyright (c) 2014, 2015 PsyPhy Consulting
///

/// Defines Windows Form to display complete text of step in the Grip script.

namespace GripMMI {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for GripMMIFullStep
	/// </summary>
	public ref class GripMMIFullStep : public System::Windows::Forms::Form
	{
	public:
		GripMMIFullStep(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~GripMMIFullStep()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::GroupBox^  groupBox1;
	public: System::Windows::Forms::TextBox^  fullStep;
	protected: 

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
			this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
			this->fullStep = (gcnew System::Windows::Forms::TextBox());
			this->groupBox1->SuspendLayout();
			this->SuspendLayout();
			// 
			// groupBox1
			// 
			this->groupBox1->Controls->Add(this->fullStep);
			this->groupBox1->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->groupBox1->Location = System::Drawing::Point(10, 9);
			this->groupBox1->Name = L"groupBox1";
			this->groupBox1->Size = System::Drawing::Size(1513, 55);
			this->groupBox1->TabIndex = 0;
			this->groupBox1->TabStop = false;
			this->groupBox1->Text = L"Selected Line in Script";
			// 
			// fullStep
			// 
			this->fullStep->Font = (gcnew System::Drawing::Font(L"Microsoft Sans Serif", 11, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
				static_cast<System::Byte>(0)));
			this->fullStep->Location = System::Drawing::Point(6, 22);
			this->fullStep->Multiline = true;
			this->fullStep->Name = L"fullStep";
			this->fullStep->Size = System::Drawing::Size(1500, 28);
			this->fullStep->TabIndex = 18;
			this->fullStep->Text = L"one two three";
			// 
			// GripMMIFullStep
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(1532, 75);
			this->Controls->Add(this->groupBox1);
			this->Name = L"GripMMIFullStep";
			this->Text = L"GripMMI Script Step";
			this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &GripMMIFullStep::GripMMIFullStep_FormClosing);
			this->groupBox1->ResumeLayout(false);
			this->groupBox1->PerformLayout();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void GripMMIFullStep_FormClosing(System::Object^  sender, System::Windows::Forms::FormClosingEventArgs^  e) {
				 Hide();
				 e->Cancel = true;
			 }
	};
}
