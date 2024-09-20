#pragma once

namespace siteCheck {

    using namespace System;
    using namespace System::ComponentModel;
    using namespace System::Collections;
    using namespace System::Windows::Forms;
    using namespace System::Data;
    using namespace System::Drawing;
    using namespace System::Net;
    using namespace System::Threading::Tasks;
    using namespace System::Timers;
    using namespace System::IO;

    public ref class MyForm : public System::Windows::Forms::Form
    {
    public:
        MyForm(void)
        {
            InitializeComponent();
            LoadSettings();
            ValidateInputs();
        }

    protected:
        ~MyForm()
        {
            if (components)
            {
                delete components;
            }
        }

    private: System::Windows::Forms::TextBox^ urlTextBox;
    private: System::Windows::Forms::TextBox^ timeTextBox;
    private: System::Windows::Forms::RichTextBox^ statusRichTextBox;
    private: System::Windows::Forms::Button^ checkButton;
    private: System::Timers::Timer^ checkTimer;

    private:
        System::ComponentModel::Container^ components;

#pragma region Windows Form Designer generated code
        void InitializeComponent(void)
        {
            this->urlTextBox = (gcnew System::Windows::Forms::TextBox());
            this->timeTextBox = (gcnew System::Windows::Forms::TextBox());
            this->statusRichTextBox = (gcnew System::Windows::Forms::RichTextBox());
            this->checkButton = (gcnew System::Windows::Forms::Button());
            this->SuspendLayout();
            // 
            // urlTextBox
            // 
            this->urlTextBox->Location = System::Drawing::Point(12, 12);
            this->urlTextBox->Name = L"urlTextBox";
            this->urlTextBox->Size = System::Drawing::Size(260, 20);
            this->urlTextBox->TabIndex = 0;
            this->urlTextBox->TextChanged += gcnew System::EventHandler(this, &MyForm::ValidateInputs);
            // 
            // timeTextBox
            // 
            this->timeTextBox->Location = System::Drawing::Point(12, 38);
            this->timeTextBox->Name = L"timeTextBox";
            this->timeTextBox->Size = System::Drawing::Size(260, 20);
            this->timeTextBox->TabIndex = 1;
            this->timeTextBox->TextChanged += gcnew System::EventHandler(this, &MyForm::ValidateInputs);
            // 
            // statusRichTextBox
            // 
            this->statusRichTextBox->Location = System::Drawing::Point(12, 64);
            this->statusRichTextBox->Name = L"statusRichTextBox";
            this->statusRichTextBox->Size = System::Drawing::Size(260, 96);
            this->statusRichTextBox->TabIndex = 2;
            this->statusRichTextBox->Text = L"";
            // 
            // checkButton
            // 
            this->checkButton->Location = System::Drawing::Point(12, 166);
            this->checkButton->Name = L"checkButton";
            this->checkButton->Size = System::Drawing::Size(260, 23);
            this->checkButton->TabIndex = 3;
            this->checkButton->Text = L"Check";
            this->checkButton->UseVisualStyleBackColor = true;
            this->checkButton->Click += gcnew System::EventHandler(this, &MyForm::checkButton_Click);
            // 
            // MyForm
            // 
            this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
            this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
            this->ClientSize = System::Drawing::Size(284, 201);
            this->Controls->Add(this->checkButton);
            this->Controls->Add(this->statusRichTextBox);
            this->Controls->Add(this->timeTextBox);
            this->Controls->Add(this->urlTextBox);
            this->Name = L"MyForm";
            this->Text = L"MyForm";
            this->FormClosing += gcnew System::Windows::Forms::FormClosingEventHandler(this, &MyForm::MyForm_FormClosing);
            this->ResumeLayout(false);
            this->PerformLayout();
        }
#pragma endregion

    private: System::Void LoadSettings() {
        String^ path = Application::StartupPath + "\\setting.ini";
        if (File::Exists(path)) {
            try {
                array<String^>^ lines = File::ReadAllLines(path);
                if (lines->Length >= 2) {
                    urlTextBox->Text = lines[0];
                    timeTextBox->Text = lines[1];
                }
                else {
                    CreateDefaultSettings(path);
                }
            }
            catch (Exception^) {
                CreateDefaultSettings(path);
            }
        }
        else {
            CreateDefaultSettings(path);
        }
    }

    private: System::Void CreateDefaultSettings(String^ path) {
        File::WriteAllText(path, "http://example.com\n60");
        urlTextBox->Text = "http://example.com";
        timeTextBox->Text = "60";
    }

    private: System::Void SaveSettings() {
        String^ url = urlTextBox->Text;
        String^ time = timeTextBox->Text;
        String^ path = Application::StartupPath + "\\setting.ini";

        // Добавляем http:// если отсутствует
        if (!url->StartsWith("http://") && !url->StartsWith("https://")) {
            url = "http://" + url;
            urlTextBox->Text = url;
        }

        // Сохраняем URL и время в setting.ini
        File::WriteAllText(path, url + "\n" + time);
    }

    private: System::Void checkButton_Click(System::Object^ sender, System::EventArgs^ e) {
        SaveSettings();
        StartTimer();
    }

    private: System::Void StartTimer() {
        double interval;
        if (Double::TryParse(timeTextBox->Text, interval)) {
            interval *= 1000; // Преобразуем секунды в миллисекунды
            checkTimer = gcnew System::Timers::Timer(interval);
            checkTimer->Elapsed += gcnew ElapsedEventHandler(this, &MyForm::OnTimedEvent);
            checkTimer->AutoReset = true;
            checkTimer->Enabled = true;
        }
        else {
            statusRichTextBox->AppendText("Ошибка: неверный формат времени\n");
        }
    }

    private: System::Void OnTimedEvent(System::Object^ source, ElapsedEventArgs^ e) {
        CheckWebsiteAsync();
    }

    private: Task^ CheckWebsiteAsync() {
        return Task::Run(gcnew Action(this, &MyForm::CheckWebsite));
    }

    private: System::Void CheckWebsite() {
        String^ url = urlTextBox->Text;

        // Проверяем доступность сайта
        try {
            HttpWebRequest^ request = dynamic_cast<HttpWebRequest^>(WebRequest::Create(url));
            request->Method = "GET";
            HttpWebResponse^ response = dynamic_cast<HttpWebResponse^>(request->GetResponse());

            if (response->StatusCode == HttpStatusCode::OK) {
                statusRichTextBox->Invoke(gcnew Action<String^>(this, &MyForm::UpdateStatus), "Сайт доступен\n");
            }
            else {
                statusRichTextBox->Invoke(gcnew Action<String^>(this, &MyForm::UpdateStatus), "Сайт недоступен\n");
            }
            response->Close();
        }
        catch (Exception^ ex) {
            statusRichTextBox->Invoke(gcnew Action<String^>(this, &MyForm::UpdateStatus), "Ошибка: " + ex->Message + "\n");
        }
    }

    private: System::Void UpdateStatus(String^ status) {
        statusRichTextBox->AppendText(status);
    }

    private: System::Void MyForm_FormClosing(System::Object^ sender, System::Windows::Forms::FormClosingEventArgs^ e) {
        if (checkTimer != nullptr) {
            checkTimer->Stop();
            checkTimer->Close(); // Используем метод Close вместо Dispose
        }
    }

    private: System::Void ValidateInputs(System::Object^ sender, System::EventArgs^ e) {
        ValidateInputs();
    }

    private: System::Void ValidateInputs() {
        double interval;
        bool isUrlValid = !String::IsNullOrWhiteSpace(urlTextBox->Text);
        bool isTimeValid = Double::TryParse(timeTextBox->Text, interval) && interval > 0;
        checkButton->Enabled = isUrlValid && isTimeValid;
    }
    };
}
