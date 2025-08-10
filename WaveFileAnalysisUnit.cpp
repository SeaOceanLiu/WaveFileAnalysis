//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "WaveFileAnalysisUnit.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMainForm *MainForm;
//---------------------------------------------------------------------------
__fastcall TMainForm::TMainForm(TComponent* Owner)
	: TForm(Owner),
    m_waveFile(nullptr)
{
}
//---------------------------------------------------------------------------
void __fastcall TMainForm::OpenButtonClick(TObject *Sender)
{
    DEBUG_PRINT("Button1Click");
	if(this->OpenFileDialog->Execute()){
		MainForm->Caption = "Wave file analysis tool [" + OpenFileDialog->FileName + "]";

		this->TreeView1->Items->Clear();

		m_waveFile = make_shared<WaveFile>(fs::path(this->OpenFileDialog->FileName.c_str()));

		this->Memo1->Lines->Add("Open file: " + this->OpenFileDialog->FileName);

		ChunkHeader* pChunkHeader =  m_waveFile->getFirstChunk();

		TTreeNode *pNode = this->TreeView1->Items->Add(nullptr, "'" + UnicodeString(pChunkHeader->ChunkID, 4) + "'");
        shared_ptr<vector<shared_ptr<ExplainationInfo>>>explaination = m_waveFile->getExplaination(pChunkHeader);

        pNode->Data = (*explaination)[0].get();

        for (int i = 1; i < explaination->size(); i++){
            TTreeNode *pSubNode = this->TreeView1->Items->AddChild(pNode, UnicodeString((*explaination)[i]->text.c_str()));
            pSubNode->Data = (*explaination)[i].get();
        }
		traverseChildChunk(pChunkHeader, pNode);
		this->TreeView1->FullExpand();

		this->Memo1->Lines->Add("    Explaination ended.");
		PlayButton->Enabled = true;
	}
}
//---------------------------------------------------------------------------
void TMainForm::traverseBrotherChunk(ChunkHeader *parentChunk, ChunkHeader *siblingChunk, TTreeNode *siblingNode)
{
    ChunkHeader *pChunkHeader = m_waveFile->getNextBrotherChunk(parentChunk, siblingChunk);
    if(pChunkHeader == nullptr){
		return;
    }
    TTreeNode *pNode = this->TreeView1->Items->Add(siblingNode, "'" + UnicodeString(pChunkHeader->ChunkID, 4) + "'");
    shared_ptr<vector<shared_ptr<ExplainationInfo>>>explaination = m_waveFile->getExplaination(pChunkHeader);
	pNode->Data = (*explaination)[0].get();

    for (int i = 1; i < explaination->size(); i++){
        TTreeNode *pSubNode = this->TreeView1->Items->AddChild(pNode, UnicodeString((*explaination)[i]->text.c_str()));
        pSubNode->Data = (*explaination)[i].get();
    }

    if(m_waveFile->isKnownChunk(pChunkHeader)){
        traverseChildChunk(pChunkHeader, pNode);
    }
    traverseBrotherChunk(parentChunk, pChunkHeader, pNode);
}
void TMainForm::traverseChildChunk(ChunkHeader *parentChunk, TTreeNode *parentNode)
{
    ChunkHeader *pChildHeader = m_waveFile->getFirstSubChunk(parentChunk);
    if(pChildHeader == nullptr){
        return;
    }
    if(m_waveFile->isKnownChunk(pChildHeader)){
        TTreeNode *pNode = this->TreeView1->Items->AddChild(parentNode, "'" + UnicodeString(pChildHeader->ChunkID, 4) + "'");
        shared_ptr<vector<shared_ptr<ExplainationInfo>>>explaination = m_waveFile->getExplaination(pChildHeader);
        pNode->Data = (*explaination)[0].get();

        for (int i = 1; i < explaination->size(); i++){
            TTreeNode *pSubNode = this->TreeView1->Items->AddChild(pNode, UnicodeString((*explaination)[i]->text.c_str()));
            pSubNode->Data = (*explaination)[i].get();
        }

        traverseBrotherChunk(parentChunk, pChildHeader, pNode);
    } else {
		this->Memo1->Lines->Add("****Under chunk: '" + UnicodeString(parentChunk->ChunkID, 4) + "'");
		this->Memo1->Lines->Add("----Unknown chunk: '" + UnicodeString(pChildHeader->ChunkID, 4) + "'");
    }
}
void __fastcall TMainForm::TreeView1Click(TObject *Sender)
{
	TreeView1->Enabled = false;
	ListView1->Enabled = false;
	ListView1->ShowColumnHeaders = true;
	TTreeNode *pSelectedNode = this->TreeView1->Selected;
	ExplainationInfo *info = (ExplainationInfo *)pSelectedNode->Data;
	ListView1->Clear();
	for (size_t i = 0; i < info->dataSize; i+=16){
		std::stringstream address;
		address << std::hex << std::setw(16) << std::setfill('0') << std::uppercase << ((uint64_t)(info->pData + i));

		TListItem* item = ListView1->Items->Add();
		item->Caption = UnicodeString(address.str().c_str());
		size_t end = (i + 16 > info->dataSize) ? info->dataSize : i + 16;
		string asciiStr;
		for (size_t j = i; j < end; j++){
			std::stringstream ss;
			ss << std::hex << std::setw(2) << std::setfill('0') << std::uppercase << int((unsigned char)(info->pData[j]));
			asciiStr += isprint(static_cast<unsigned char>(info->pData[j])) ? info->pData[j] : '.';

			item->SubItems->Add(UnicodeString(ss.str().c_str()));
		}
		for (size_t j = end; j < i + 16; j++){
			item->SubItems->Add(UnicodeString(""));
		}
		item->SubItems->Add(UnicodeString(asciiStr.c_str()));

		// std::stringstream ss;
		Application->ProcessMessages(); // 这里需要改为使用线程逻辑来保证，不然直接这么调用会导致ListView中存储的数据在重选择时无法被清除
	}
	ListView1->Enabled = true;
	TreeView1->Enabled = true;
}
//---------------------------------------------------------------------------




void __fastcall TMainForm::PlayButtonClick(TObject *Sender)
{
	DEBUG_PRINT("PlayButton clicked");
    if(m_waveFile == nullptr){
        return;
    }
    ChunkHeader *pChunkHeader;

    pChunkHeader = m_waveFile->getChunk("data");
    m_wavePlayer = make_shared<WavePlayer>((char *)pChunkHeader + CHUNK_HEADER_SIZE, (size_t)pChunkHeader->ChunkSize);

    pChunkHeader = m_waveFile->getChunk("fmt ");
    FormatChunk *pFormatChunk = (FormatChunk *)(pChunkHeader);

    WAVEFORMATEXTENSIBLE waveFormatExtensible;
    waveFormatExtensible.Format.wFormatTag = pFormatChunk->audioFormat;
    waveFormatExtensible.Format.nChannels = pFormatChunk->numChannels;
    waveFormatExtensible.Format.nSamplesPerSec = pFormatChunk->sampleRate;
    waveFormatExtensible.Format.nAvgBytesPerSec = pFormatChunk->byteRate;
    waveFormatExtensible.Format.nBlockAlign = pFormatChunk->blockAlign;
    waveFormatExtensible.Format.wBitsPerSample = pFormatChunk->bitsPerSample;
    waveFormatExtensible.Format.cbSize = 0;

    if (pFormatChunk->header.ChunkSize > sizeof(FormatChunk) - CHUNK_HEADER_SIZE)
    {
        if (pFormatChunk->audioFormat == static_cast<uint16_t>(WaveFormat::EXTENSIBLE)) {
            Extension *pFormatExtension = (Extension *)pFormatChunk->pData;
            waveFormatExtensible.Format.cbSize = pFormatExtension->extSize;
            if (pFormatExtension->extSize == 22 && pFormatChunk->audioFormat == static_cast<uint16_t>(WaveFormat::EXTENSIBLE)){
                WaveFormatExt *pWaveFormatExt = (WaveFormatExt *)(pFormatExtension->pData);

                waveFormatExtensible.Samples.wValidBitsPerSample = pWaveFormatExt->validBitsPerSample;
                waveFormatExtensible.dwChannelMask = pWaveFormatExt->channelMask;
                waveFormatExtensible.SubFormat.Data1 = pWaveFormatExt->subFormat.Data1;
                waveFormatExtensible.SubFormat.Data2 = pWaveFormatExt->subFormat.Data2;
                waveFormatExtensible.SubFormat.Data3 = pWaveFormatExt->subFormat.Data3;
                waveFormatExtensible.SubFormat.Data4[0] = pWaveFormatExt->subFormat.Data4[0];
                waveFormatExtensible.SubFormat.Data4[1] = pWaveFormatExt->subFormat.Data4[1];
                waveFormatExtensible.SubFormat.Data4[2] = pWaveFormatExt->subFormat.Data4[2];
                waveFormatExtensible.SubFormat.Data4[3] = pWaveFormatExt->subFormat.Data4[3];
                waveFormatExtensible.SubFormat.Data4[4] = pWaveFormatExt->subFormat.Data4[4];
                waveFormatExtensible.SubFormat.Data4[5] = pWaveFormatExt->subFormat.Data4[5];
                waveFormatExtensible.SubFormat.Data4[6] = pWaveFormatExt->subFormat.Data4[6];
                waveFormatExtensible.SubFormat.Data4[7] = pWaveFormatExt->subFormat.Data4[7];
            }
        }

    }

    m_wavePlayer->setFormat(&waveFormatExtensible);
    m_wavePlayer->play();
    PlayButton->Enabled = false;
    PauseResumeButton->Enabled = true;
    StopButton->Enabled = true;
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::PauseResumeButtonClick(TObject *Sender)
{
	DEBUG_PRINT("PauseResumeButton clicked");
	if (m_wavePlayer == nullptr) {
        return;
    }
    if (m_wavePlayer->isPlaying()) {
        m_wavePlayer->pause();
    } else {
        m_wavePlayer->resume();
	}
}
//---------------------------------------------------------------------------

void __fastcall TMainForm::StopButtonClick(TObject *Sender)
{
	DEBUG_PRINT("StopButton clicked");
	if (m_wavePlayer == nullptr) {
        return;
    }
    m_wavePlayer->stop();
}
//---------------------------------------------------------------------------
UnicodeString TMainForm::getErrorMessage(HRESULT hr){
    if (m_wavePlayer == nullptr){
        return "WavePlayer is not initialized";
    }
    return UnicodeString(m_wavePlayer->getErrorString(hr).c_str());
}
void __fastcall TMainForm::EventTimerTimer(TObject *Sender)
{
	shared_ptr<Event> eventInQueue = EventQueue::getInstance()->popEventFromQueue();

	while(eventInQueue != nullptr){
		DEBUG_STRING("EventTimerTimer: " + to_string(static_cast<int>(eventInQueue->m_eventName)));
		switch(eventInQueue->m_eventName){
		case EventName::Playing:
            this->PauseResumeButton->Caption = "暂停(P&ause)";
            this->StopButton->Enabled = true;
            this->PauseResumeButton->Enabled = true;
            this->PlayButton->Enabled = false;
			break;
		case EventName::Error:
            this->Memo1->Lines->Add("Playing error: " + getErrorMessage(any_cast<HRESULT>(eventInQueue->m_eventParam)));
            this->PauseResumeButton->Caption = "暂停(P&ause)";
            this->StopButton->Enabled = true;
            this->PauseResumeButton->Enabled = true;
            this->PlayButton->Enabled = false;
			break;
		case EventName::Paused:
            this->PauseResumeButton->Caption = "恢复(&Resume)";
            this->StopButton->Enabled = true;
            this->PauseResumeButton->Enabled = true;
            this->PlayButton->Enabled = false;
			break;
		case EventName::Resumed:
            this->PauseResumeButton->Caption = "暂停(P&ause)";
            this->StopButton->Enabled = true;
            this->PauseResumeButton->Enabled = true;
            this->PlayButton->Enabled = false;
			break;
		case EventName::Stop:
            this->PauseResumeButton->Caption = "暂停(P&ause)";
            this->StopButton->Enabled = false;
            this->PauseResumeButton->Enabled = false;
            this->PlayButton->Enabled = true;
			break;
		default:
            break;
		}
		// 改为使用shared_ptr后，不需要手动释放eventInQueue内存
		eventInQueue = EventQueue::getInstance()->popEventFromQueue();
	}
}
//---------------------------------------------------------------------------

