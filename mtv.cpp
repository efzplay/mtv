#include "define.h"
#include "easy2use.h"
#include "draw2d.h"
#include "soundmgr.h"
#include "datamgr.h"
#include "UImanager.h"
#include "taskmgr.h"
#include "mtv.h"
#include "gameloop.h"
//#include "lyricsUnit.h"

#include <locale.h>111

#define FMOD_UPDATE_DELAY (150)

enum
{
#include "media\MTV_title.idx"
TITLE_BGM_NUM_MAX
};

enum
{
#include "inc\fate.idx"
FATE_TEX_MAX
};

enum
{
#include "inc\nadeko.idx"
NADEKO_TEX_MAX
};

extern LPDIRECT3DDEVICE9        g_pd3dDevice;
static int test_texno[MTV_NUM_MAX + 1] = {1,2,3};
static MyMTV* g_pMTV = NULL;
//static std::vector<POINT> g_whList;

MyMTV::MyMTV()
{
	m_MTVMode = MTV_INIT;
	m_nMTVIndex = 0;
	srand((unsigned) time(NULL)); 
	m_nTitleMusicIndex = rand()%TITLE_BGM_NUM_MAX;
	m_BGMStep = 1;
	m_BGMTimer = 0;
	m_nLoad = 0;
	m_nMTVLoad = 0;
	m_nMusicLength = 0;
	m_nMusicPositon = 0;
	m_nMusicLoopCount = 0;
	m_nGraphicNumber = 0;
	m_LycIndex = 0;
	m_bMusicPaused = false;
	Datamgr_LoadIDX(MTVMusicNameList,L"media\\MTV_BGM.idx");
	g_pMTV = this;
	m_MTVpreview = NULL;
	m_button_pause = NULL;
#ifndef LOAD_UI_FROM_FILE
	m_background = (CPanel*)CreateUI(NULL,"MTVtitle_bg",GUI_TYPE_PANEL,0,0,DISPLAY_WIDTH,DISPLAY_HIGHT);
	m_MTVbackground = (CPanel*)CreateUI(NULL,"MTV_bg",GUI_TYPE_PANEL,0,0,DISPLAY_WIDTH,DISPLAY_HIGHT);
	m_MTVbackground->SetVisible(false);
	for(int i = 0;i < MTV_NUM_MAX;i++)
	{
		char name[20] = {0};
		sprintf_s(name,"MTVtitle_button_%d",i);
		m_MTVbutton[i] = (CUIButton*)CreateUI(m_background,name,GUI_TYPE_BUTTON,30.f ,30.f + i*110,100,100);
		//m_MTVbutton[i]->SetTexPosX(100*i);
		//m_MTVbutton[i]->SetTexPosY(300+20*i);
	}
	m_MTVtitle = (CUIFont*)CreateUI(m_background,"MTVtitle",GUI_TYPE_FONT,660.f,360.f,400,30);
	m_MTVtitle->SetColor(0xffff0000);
	InitFont(m_MTVtitle,L"");
	m_MTVtitle->SetVisible(false);
	m_button_exit = (CUIButton*)CreateUI(m_background,"MTVexit",GUI_TYPE_BUTTON,60.f ,700.f,60,20);
	m_button_pause = (CUIButton*)CreateUI(m_MTVbackground,"MTVpause",GUI_TYPE_BUTTON,60.f ,670.f,60,20);
	m_button_back = (CUIButton*)CreateUI(m_MTVbackground,"MTVback",GUI_TYPE_BUTTON,60.f ,600.f,60,20);
	m_MTVsub = (CUIFont*)CreateUI(m_MTVbackground,"MTVsub",GUI_TYPE_FONT,150.f,670.f,1000,40);
	m_MTVsub->SetVisible(false);
	InitFont(m_MTVsub,L"");
#endif
#ifdef USE_SCRIPT_FLOW
	m_SubFlowControlfile = 0;
#endif
}

MyMTV::~MyMTV()
{
	SoundMgr_StopChannel(FMOD_CHANNEL_BGM);
	for (UINT i = 0;i < MTVMusicList.size();i++)
	{
		SoundMgr_ReleaseSound(MTVMusicList[i]);
	}
	for (UINT i = 0;i < TitleMusicList.size();i++)
	{
		SoundMgr_ReleaseSound(TitleMusicList[i]);
	}
	TitleMusicList.clear();
	DestroyUI(m_background);
	DestroyUI(m_MTVbackground);
	g_pMTV = NULL;
// 	if(m_SubFlowControlfile!=0){
// 		fclose(m_SubFlowControlfile);
// 		m_SubFlowControlfile = 0;
// 	}
}

void MyMTV::Main()
{
	switch (m_MTVMode)
	{
	case MTV_INIT:
		if(LoadData())
		{
			m_MTVMode = MTV_TITLE;
		}
		break;
	case MTV_TITLE:
		if(m_bMusicPaused) break;
		UpdateTitleMusic();
		break;
	case MTV_LOAD:
		if(LoadMTVData())
		{
			m_MTVMode = MTV_LOOP;
		}
		break;
	case MTV_LOOP:
		if(m_bMusicPaused) break;
#ifdef USE_SCRIPT_FLOW
		if((GetKeyState(VK_LBUTTON) & 0x80) != 0 && m_isPressing == false){
			saveTimeEvent();
			m_isPressing = true;
		}else if((GetKeyState(VK_LBUTTON) & 0x80) == 0){
			// when the ESC key gets let go, set the flag back to false;
			m_isPressing = false;
		}
#endif
#ifdef USE_FMOD
		SoundMgr_GetMusicPostion(FMOD_CHANNEL_BGM,&m_nMusicPositon);
#endif
		UpdateSubTitle();
		ChangeBackground();
		break;
	case MTV_OVER:
		ReturnToTitle();
		break;
	default:
		break;
	}
}

void MyMTV::Draw()
{
	switch (m_MTVMode)
	{
	case MTV_TITLE:
	case MTV_LOOP:
		RenderUI();
		break;
	default:
		break;
	}
}

bool MyMTV::LoadData()
{
	switch(m_nLoad)
	{
	case 0:
		{
			if(Datamgr_LoadAllTexture(m_TexMap[L"MTV_title"],L"MTV_title"))
			{
#ifndef LOAD_UI_FROM_FILE
				for(int i = 0;i < MTV_NUM_MAX;i++)
				{
					m_MTVbutton[i]->AddTexture(m_TexMap[L"MTV_title"][test_texno[i]],test_texno[i],L"MTV_title");
				}
				m_background->AddTexture(m_TexMap[L"MTV_title"][0],0,L"MTV_title");
#endif
				m_nLoad++;
			}
		}
		break;
	case 1:
		{
			//todo commonUI加载逻辑不应该放在这里
			if(Datamgr_LoadAllTexture(m_TexMap[L"common"],L"common"))
			{
#ifndef LOAD_UI_FROM_FILE
					m_button_back->AddTexture(m_TexMap[L"common"][0],0,L"common");
					m_button_exit->AddTexture(m_TexMap[L"common"][1],1,L"common");
#endif
				m_nLoad++;
			}
		}
	case 2:
		{
#ifdef USE_FMOD
			if(SoundMgr_LoadAll(TitleMusicList,L"media\\MTV_title.qb"))
#endif
			{
				m_bMusicPaused = false;
#ifdef USE_FMOD
				SoundMgr_PlaySound(TitleMusicList[m_nTitleMusicIndex].sound,FMOD_CHANNEL_BGM,false);
				SoundMgr_GetMusicLength(TitleMusicList[m_nTitleMusicIndex].sound,&m_nMusicLength);
#endif
				m_nLoad++;
			}
		}
		break;
	case 3:
		{
#ifdef LOAD_UI_FROM_FILE
			CString data;
			Datamgr_LoadXML(data,L"layout\\GAME_Layout.qbw",g_GameLayOutMap[g_GameModeName[GAME_MTV]]);
			QBGUICreateUIFromFile(data,m_TexMap);
			m_background = (CPanel*)QBGUIGetElement("MTVtitle_bg",GUI_TYPE_PANEL);
			m_MTVbackground = (CPanel*)QBGUIGetElement("MTV_bg",GUI_TYPE_PANEL);
			m_MTVbackground->SetVisible(false);
			for(int i = 0;i < MTV_NUM_MAX;i++)
			{
				char name[20] = {0};
				sprintf_s(name,"MTVtitle_button_%d",i);
				m_MTVbutton[i] = (CUIButton*)QBGUIGetElement(name,GUI_TYPE_BUTTON);
			}
			m_MTVtitle = (CUIFont*)QBGUIGetElement("MTVtitle",GUI_TYPE_FONT);
			m_MTVtitle->SetColor(0xffff0000);
			m_button_exit = (CUIButton*)QBGUIGetElement("MTVexit",GUI_TYPE_BUTTON);
			//m_button_pause = (CUIButton*)QBGUIGetElement("MTVpause",GUI_TYPE_BUTTON);
			m_button_back = (CUIButton*)QBGUIGetElement("MTVback",GUI_TYPE_BUTTON);
			m_MTVsub = (CUIFont*)QBGUIGetElement("MTVsub",GUI_TYPE_FONT);
			//m_MTVsub->SetTextLocation(DT_CENTER|DT_VCENTER);
#ifdef USE_SCRIPT_FLOW
			m_MTVsubFlow = (CUIFont*)QBGUIGetElement("MTVsubFlow",GUI_TYPE_FONT);
			m_MTVsubFlow->SetTextColor(0xff0000ff);
#endif
			m_MTVsubBG =  (CPanel*)QBGUIGetElement("MTV_subtitle_bg",GUI_TYPE_PANEL);
			m_MTVsubBG->SetColor(0xff000000);
#endif
			SetUICallBack(MTVCallBack);
			return true;
		}
		break;
	}
	return false;
}

bool MyMTV::LoadMTVData()
{
	switch(m_nMTVLoad)
	{
	case 0:
		{
			std::vector<LPDIRECT3DTEXTURE9> m_vTexList;
			if(Datamgr_LoadAllTexture(m_vTexList,MTVMusicNameList[m_nMTVIndex].GetBuffer()))
			{
				m_MTVbackground->ClearTexture();
				for(UINT i = 0;i < m_vTexList.size();i++)
				{
					m_MTVbackground->AddTexture(m_vTexList[i],i,MTVMusicNameList[m_nMTVIndex].GetBuffer());
				}
				m_nMTVLoad++;
			}
		}
		break;
	case 1:
		{
#ifdef USE_FMOD
			if(SoundMgr_LoadAll(MTVMusicList,L"media\\MTV_BGM.qb"))
#endif
			{
				m_nMTVLoad++;
			}
			break;
		}
	case 2:
		{
			m_bMusicPaused = false;
#ifdef USE_FMOD
			SoundMgr_PlaySound(MTVMusicList[m_nMTVIndex].sound,FMOD_CHANNEL_BGM,false);
			SoundMgr_GetMusicLength(MTVMusicList[m_nMTVIndex].sound,&m_nMusicLength);
#endif
			return true;
		}
		break;
	}
	return false;
}

void MyMTV::PlayMTV(int no)
{
	m_nMTVIndex = no;
	m_MTVMode = MTV_LOAD;
	m_BGMStep = 1;
	m_nMTVLoad = 0;
#ifdef USE_FMOD
	SoundMgr_StopChannel(FMOD_CHANNEL_BGM);
#endif
	m_bMusicPaused = true;
	switch(m_nMTVIndex)
	{
	case 0:
		m_nGraphicNumber = FATE_TEX_MAX;
		break;
	case 2:
		m_nGraphicNumber = NADEKO_TEX_MAX;
		break;
	default:
		m_nGraphicNumber = 1;
		break;
	}
	m_background->SetVisible(false);
	m_MTVbackground->SetVisible(true);
	m_MTVbackground->SetTexNo(m_BGMStep - 1);
	CString strLyc;
	Datamgr_LoadTXT(strLyc,L"script\\MTV_Lyc.qb",m_nMTVIndex);
	ReadLyc(strLyc);

#ifdef USE_SCRIPT_FLOW

	// initialize the lyrics data
	m_accurateLyricsIndex = 0;
	// read data from file.
	FILE* fp;
	int length = 0;
	char* pchBuf = NULL;
	_wfopen_s(&fp, L"script2.txt", L"r");
	fseek(fp, 0, SEEK_END);
	length = ftell(fp);
	rewind(fp);
	pchBuf = (char*) malloc(sizeof(char)*length+1);
	if(!pchBuf) {    
		perror("no memory available!\n");  
		exit(0);  
	}
	length = fread(pchBuf, sizeof(char), length, fp);
	pchBuf[length] = '\0';
	// convert data to CStirng
	CString lyrics = CA2W(pchBuf,CHARSET_GBK);

	decodeDataToLyricUnit(lyrics);

	fclose(fp);
	free(pchBuf);

	_wfopen_s(&m_SubFlowControlfile, L"script.txt", L"wt");
	// initialize the flag to FALSE.
	m_isPressing = false;

#endif
	//Datamgr_UpdateTexwhList(g_whList);
}

void MyMTV::ChangeBackground()
{
	if(m_nMusicPositon >= (int)(m_nMusicLength* m_BGMStep/m_nGraphicNumber))
	{
		m_BGMStep++;
		if(m_BGMStep > m_nGraphicNumber) 
		{
			m_BGMStep = m_nGraphicNumber;
		}
		else
		{
			m_MTVbackground->SetTexNo(m_BGMStep - 1);
		}
	}
}

void MyMTV::ReadLyc(CString strLyc)
{
	m_LycWaitTime.clear();
	m_LycText.clear();
	m_LycIndex = 0;
	int line = 0;
	CString time;
	CString  szLine;
	int strLineEndPos = strLyc.Find(L'\n');
	//szLine = strLyc.Left(strLineEndPos + 1);
	while( strLineEndPos != -1 )
	{
		szLine = strLyc.Left(strLineEndPos + 1);
		int start = szLine.Find('[');
		int end = szLine.Find(']');
		int hour_end =  szLine.Find('.');
		UINT tmpWaitTime = 0;
		for (int i = 0;i < 3;i++)
		{
			time = szLine.Mid(start + 1 +3*i,start + 2 + i*3);
			switch (i)//等待时间计算
			{
			case 0:
				tmpWaitTime += StrToInt(time) * 60 * 1000;
				break;
			case 1:
				tmpWaitTime += StrToInt(time) * 1000;
				break;
			case 2:
				tmpWaitTime += StrToInt(time) * 10;
				break;
			}
		}
		m_LycWaitTime.push_back(tmpWaitTime);
		szLine.Delete(start,end - start + 1);
		m_LycText.push_back(szLine);
		strLyc = strLyc.Right(strLyc.GetLength() - strLineEndPos - 1);
		strLineEndPos = strLyc.Find(L'\n');
		line++;
	}
	m_MTVsub->SetText(m_LycText[0].GetBuffer());
#ifdef USE_SCRIPT_FLOW
	m_MTVsubFlow->SetText(m_LycText[0].GetBuffer());
	m_striptOriginalWidth = QBGUI_GetTextLength( m_MTVsubFlow->GetText());
	m_accurateLyricsCount = 0;
	m_nextPosition = 0;
	m_accurateLyricsIndex = 0;
	m_previousPosition = 0;
	m_previousAccurateLyricsIndex = 0;

	m_PreLycIndex = 0;
#endif
}

void MyMTV::UpdateSubTitle()
{
	if(m_nMusicPositon >= m_nMusicLength - FMOD_UPDATE_DELAY)
	//	|| (m_nMusicPositon <= 10000 && m_LycIndex == (int)m_LycWaitTime.size() - 1))
	{
#ifdef USE_FMOD
		SoundMgr_StopChannel(FMOD_CHANNEL_BGM);
#endif
		m_bMusicPaused = true;
		m_MTVsub->SetText(L"");
		m_LycIndex = 0;
#ifdef USE_SCRIPT_FLOW
		m_MTVsubFlow->SetText(L"");
#endif
	}
	else if (m_LycIndex < (int)m_LycWaitTime.size() - 1)
	{
		if(m_nMusicPositon >= m_LycWaitTime[m_LycIndex] && m_nMusicPositon < m_LycWaitTime[m_LycIndex+1])
		{
			//todo 字幕流动
			updateLyricsProcess();
		}
		else
		{
			m_LycIndex++;
			m_MTVsub->SetText(m_LycText[m_LycIndex].GetBuffer());
#ifdef USE_SCRIPT_FLOW
			m_PreLycIndex = 0;
			CString data = "\n";
			fwrite(data.GetBuffer(), sizeof(char), data.GetLength(), m_SubFlowControlfile); 

			// new lyric line, reset all the variables
			m_MTVsubFlow->SetText(m_LycText[m_LycIndex].GetBuffer());
			m_striptOriginalWidth = QBGUI_GetTextLength( m_MTVsubFlow->GetText());
			m_accurateLyricsCount = 0;
			m_nextPosition = 0;
			m_previousAccurateLyricsIndex = 0;
			m_previousPosition = 0;


#endif
		}
	}
}

void MyMTV::UpdateTitleMusic()
{
#ifdef USE_FMOD
	SoundMgr_GetMusicPostion(FMOD_CHANNEL_BGM,&m_nMusicPositon);
	if(m_nMusicPositon >= m_nMusicLength - FMOD_UPDATE_DELAY)
	{
		SoundMgr_StopChannel(FMOD_CHANNEL_BGM);
		m_nTitleMusicIndex++;
		if(m_nTitleMusicIndex > TITLE_BGM_NUM_MAX - 1)
		{
			m_nTitleMusicIndex = 0;
		}
		SoundMgr_GetMusicLength(TitleMusicList[m_nTitleMusicIndex].sound,&m_nMusicLength);
		SoundMgr_PlaySound(TitleMusicList[m_nTitleMusicIndex].sound,FMOD_CHANNEL_BGM,false);
	}
#endif
}

void MyMTV::BackToSelect()
{
	srand((unsigned) time(NULL)); 
	m_nTitleMusicIndex = rand()%TITLE_BGM_NUM_MAX;
	m_bMusicPaused = false;
#ifdef USE_FMOD
	for (UINT i = 0;i < MTVMusicList.size();i++)
	{
		SoundMgr_ReleaseSound(MTVMusicList[i]);
	}
	MTVMusicList.clear();
	//SoundMgr_StopChannel(FMOD_CHANNEL_BGM);
	SoundMgr_PlaySound(TitleMusicList[m_nTitleMusicIndex].sound,FMOD_CHANNEL_BGM,false);
	SoundMgr_GetMusicLength(TitleMusicList[m_nTitleMusicIndex].sound,&m_nMusicLength);
#endif
	m_MTVMode = MTV_TITLE;
	m_nMTVIndex = 0;
	m_background->SetVisible(true);
	m_MTVbackground->SetVisible(false);
	m_background->SetEnable(true);
	m_MTVtitle->SetText(L"");

#ifdef USE_SCRIPT_FLOW
	if(m_SubFlowControlfile!=0){
		fclose(m_SubFlowControlfile);
		m_SubFlowControlfile = 0;
	}
#endif
}

void MyMTV::ShowMTVDetail(bool flg)
{
	m_MTVtitle->SetVisible(flg);
	m_MTVtitle->SetText(MTVMusicNameList[m_nMTVIndex].GetBuffer());
}

void CALLBACK MyMTV::MTVCallBack( unsigned int nEvent, CElement* sender)
{
	if(g_pMTV == NULL) return;
	switch(nEvent)
	{
	case EVENT_BUTTON_CLICKED:
		{
			for(int i = 0;i < MTV_NUM_MAX;i++)
			{
				if(sender == g_pMTV->m_MTVbutton[i])
				{
					g_pMTV->PlayMTV(i);
					return;
				}
			}
			if(sender == g_pMTV->m_button_exit)
			{
				g_pMTV->m_MTVMode = MTV_OVER;
				//ReturnToTitle();
				return;
			}
			if(sender == g_pMTV->m_button_back)
			{
				g_pMTV->BackToSelect();
				return;
			}
		}
		break;
	case EVENT_MOUSE_ENTER:
		{
			for(int i = 0;i < MTV_NUM_MAX;i++)
			{
				if(sender == g_pMTV->m_MTVbutton[i])
				{
					g_pMTV->m_nMTVIndex = i;
					g_pMTV->ShowMTVDetail(true);
					return;
				}
			}
		}
		break;
	case EVENT_MOUSE_LEAVE:
		{
			for(int i = 0;i < MTV_NUM_MAX;i++)
			{
				if(sender == g_pMTV->m_MTVbutton[i])
				{
					g_pMTV->ShowMTVDetail(false);
					return;
				}
			}
		}
	default:
		break;
	}
}

void MyMTV::saveTimeEvent(){
	if(g_MousePoint.y < m_MTVsubFlow->GetPosY() || g_MousePoint.y > (m_MTVsubFlow->GetPosY() + m_MTVsubFlow->GetHeight())){
		return;
	}

	CString character, partition, position;
	position.Format(L"%d", m_nMusicPositon);
	/*
	int p = g_MousePoint.x - m_MTVsubFlow->GetPosX();
	p = p/(m_striptOriginalWidth/(m_MTVsubFlow->GetText().GetLength()));
	*/
	int p = findLetterIndex((float)g_MousePoint.x);
	partition.Format(L"%d", p);
	character = m_MTVsubFlow->GetText().Mid(p, 1);

	CString data = //index + "@" +  
		character + L"@" + partition + L"#" + position + L",";
	
	char* tmp = new char[GetStringAscLen(data)];
	int r = WideCharToMultiByte(CHARSET_GBK,0,data.GetBuffer() ,data.GetLength(),tmp, GetStringAscLen(data), NULL, NULL);
	fwrite(tmp, sizeof(char), GetStringAscLen(data), m_SubFlowControlfile); 
	delete tmp;
}

// returns the position of the letter clicked.
int MyMTV::findLetterIndex(float x){
	int p = (int)x - (int)m_MTVsubFlow->GetPosX();
	int length, r = 0;
	CString lyrics = m_MTVsubFlow->GetText();
	for(int i = 0; i < lyrics.GetLength(); i++){
		length = QBGUI_GetTextLength(lyrics.Left(i+1));
		if((length - p) > 0){
			r = i;
			break;
		}
	}
	return r;
}

void MyMTV::decodeDataToLyricUnit(CString str){

	int lineIdx = 0, parIdx, pLen, lineCut;
	CString unit;
	std::vector<UINT32> vector;
	CString lineStr;
	//str = str.Left(str.GetLength()-1);

	// format character@partition#time
	while((lineCut = str.Find('\n')) != -1){
		lineStr = str.Left(lineCut);

		int idx = 0;
		while((idx = lineStr.Find(',')) != -1){
			unit = lineStr.Left(idx);
			pLen = unit.Find('#') - unit.Find('@') - 1;
			parIdx = _ttoi(unit.Mid(unit.Find('@')+1, pLen));
			m_LycWordmap[lineIdx][parIdx] = _ttoi(unit.Right(unit.GetLength() - 1 - unit.Find('#')));

			// cut the old string
			lineStr = lineStr.Right(lineStr.GetLength() - 1 - idx);
		}
		str = str.Right(str.GetLength() - 1 - lineCut);
		lineIdx++;
	}

}


void MyMTV::updateLyricsProcess(){
#ifdef USE_SCRIPT_FLOW
		float percentage;

		// 如果存在下一个字的时间数据
		if(m_nextPosition > 0){
			// 下一个字的时间大于现在时间
			if(m_nextPosition >= m_nMusicPositon){
				UINT32 startPostion;
				// 如果存在上一个字的时间数据
				if(m_previousPosition > 0){
					startPostion = m_previousPosition;
				}else{
					startPostion = m_LycWaitTime[m_LycIndex];
				}

				percentage = ((float)m_nMusicPositon - startPostion)/(m_nextPosition - startPostion);
				if(m_previousAccurateLyricsIndex > 0){
					m_MTVsubFlow->SetWidth((UINT)(QBGUI_GetTextLength(m_MTVsubFlow->GetText().Left(m_previousAccurateLyricsIndex)) // length to the start position
						+ QBGUI_GetTextLength(m_MTVsubFlow->GetText().Mid(m_previousAccurateLyricsIndex, m_accurateLyricsIndex - m_previousAccurateLyricsIndex)) * percentage	// 		
						));
				}else{
					m_MTVsubFlow->SetWidth((UINT)(QBGUI_GetTextLength(m_MTVsubFlow->GetText().Left(m_accurateLyricsIndex)) * percentage));
				}
			}else{
				// 下一个字的时间已经无效
				m_previousPosition = m_nMusicPositon;
				m_nextPosition = 0;
				m_previousAccurateLyricsIndex = m_accurateLyricsIndex;
				
				// 以上一个字作为起点，计算歌词显示长度
				//percentage = ((float)m_nMusicPositon - m_previousPosition)/(m_LycWaitTime[m_LycIndex+1] - m_previousPosition);
				//m_MTVsubFlow->SetWidth((UINT)(QBGUI_GetTextLength(m_MTVsubFlow->GetText().Left(m_previousAccurateLyricsIndex)) // length to the start position
				//	+ (m_striptOriginalWidth - QBGUI_GetTextLength(m_MTVsubFlow->GetText().Left(m_previousAccurateLyricsIndex))) * percentage	// 		
				//	));
			
			}
		}else{
			// 不存在下一个字的时间数据

			// 检查的序号尚在该行歌词字数以内
			if(m_accurateLyricsCount <= m_MTVsubFlow->GetText().GetLength()){

				// 如果对应序号的字，有时间数据
				if(m_LycWordmap[m_LycIndex].find(m_accurateLyricsCount) != m_LycWordmap[m_LycIndex].end()){
					m_nextPosition = m_LycWordmap[m_LycIndex][m_accurateLyricsCount];
					m_accurateLyricsIndex = m_accurateLyricsCount;
					if(m_nextPosition < m_nMusicPositon){
						// 该字的时间数据已经小于播放时间，数据已经无效
						m_previousPosition = m_nMusicPositon;
						m_nextPosition = 0;
						m_previousAccurateLyricsIndex = m_accurateLyricsIndex;
						m_accurateLyricsCount++;
					}
				}else{
						m_accurateLyricsCount++;
				}
			}

			if(m_previousAccurateLyricsIndex > 0){
				// 如果前一个字存在时间数据，以上一个字作为起点，计算歌词显示长度
				percentage = ((float)m_nMusicPositon - m_previousPosition)/(m_LycWaitTime[m_LycIndex+1] - m_previousPosition);
					m_MTVsubFlow->SetWidth((UINT)(QBGUI_GetTextLength(m_MTVsubFlow->GetText().Left(m_previousAccurateLyricsIndex)) // length to the start position
					+ (m_striptOriginalWidth - QBGUI_GetTextLength(m_MTVsubFlow->GetText().Left(m_previousAccurateLyricsIndex))) * percentage	// 		
					));
			}else{
				percentage = ((float)m_nMusicPositon - m_LycWaitTime[m_LycIndex])/(m_LycWaitTime[m_LycIndex+1] - m_LycWaitTime[m_LycIndex]);

				m_MTVsubFlow->SetWidth((UINT)(m_striptOriginalWidth * percentage));
			}

		}

#endif

}