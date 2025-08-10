//---------------------------------------------------------------------------

#ifndef WaveFileAnalysisUnitH
#define WaveFileAnalysisUnitH
#include <System.Classes.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>
#include <Vcl.StdCtrls.hpp>
//---------------------------------------------------------------------------
#include <cctype>
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <Vcl.ComCtrls.hpp>
#include <Vcl.Dialogs.hpp>
#include <Vcl.ExtCtrls.hpp>

#include "DebugTrace.h"
#include "waveFormat.h"
#include "WavePlayer.h"
#include "EventQueue.h"
extern DebugTrace *pDTInst;
//---------------------------------------------------------------------------
class TMainForm : public TForm
{
__published:	// IDE-managed Components
	TTreeView *TreeView1;
	TButton *OpenButton;
	TOpenDialog *OpenFileDialog;
	TMemo *Memo1;
	TSplitter *Splitter1;
	TSplitter *Splitter2;
	TPanel *Panel1;
	TListView *ListView1;
	TButton *PlayButton;
	TButton *PauseResumeButton;
	TButton *StopButton;
	TTimer *EventTimer;
	void __fastcall OpenButtonClick(TObject *Sender);
	void __fastcall TreeView1Click(TObject *Sender);
	void __fastcall PlayButtonClick(TObject *Sender);
	void __fastcall PauseResumeButtonClick(TObject *Sender);
	void __fastcall StopButtonClick(TObject *Sender);
	void __fastcall EventTimerTimer(TObject *Sender);
private:	// User dec
	shared_ptr<WaveFile> m_waveFile;
	shared_ptr<WavePlayer> m_wavePlayer;

	void traverseBrotherChunk(ChunkHeader *parentChunk, ChunkHeader *siblingChunk, TTreeNode *siblingNode);
	void traverseChildChunk(ChunkHeader *parentChunk, TTreeNode *parentNode);
	UnicodeString getErrorMessage(HRESULT hr);

public:		// User declarations
	__fastcall TMainForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TMainForm *MainForm;
//---------------------------------------------------------------------------
#endif
