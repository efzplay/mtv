#ifndef _MTV_H_
#define _MTV_H_
#include "taskmgr.h"

enum MTV_STATUS{
	MTV_INIT,
	MTV_TITLE,
	MTV_LOOP,
	MTV_LOAD,
	MTV_OVER,
};

enum MTV_MODE{
	MTV_NO_1,
	MTV_NO_2,
};

#define MTV_NUM_MAX (3)

class Draw2DTexture;
struct lyricsUnit;

class MyMTV : public BaseTask
{
public:
	MyMTV();
	virtual ~MyMTV();
	//bool InitMusicEnv();
	virtual bool LoadData();
	bool LoadMTVData();
	virtual void Main();
	virtual void Draw();
	void PlayMTV(int no);
	void ReadLyc(CString strLyc);
	void ChangeBackground();
	void ShowMTVDetail(bool flg);
	void BackToSelect();
	wchar_t* GetTitleName(){return m_TitleName;};
	void UpdateSubTitle();
	void UpdateTitleMusic();
	static void CALLBACK MTVCallBack( unsigned int nEvent, CElement* sender);
private:
	int m_MTVMode;
	int m_nMTVIndex;
	int m_nTitleMusicIndex;
	int m_BGMStep;
	int m_BGMTimer;
	//int m_nLoad;
	int m_nMTVLoad;
	bool m_bMusicPaused;
//	int m_nLoadNextTex;
// 	int m_nMinute;
// 	int m_nSecond;
	UINT32 m_nMusicLength;
	UINT32 m_nMusicPositon;
	UINT32 m_nMusicLoopCount;
	int m_nGraphicNumber;
	std::map<UINT,CString>	MTVMusicNameList;
	std::vector<MyMusicData> TitleMusicList;//开始画面的音乐文件列表
	std::vector<MyMusicData> MTVMusicList;//MTV音乐文件列表
	std::vector<CString>  m_LycText;//每行的歌词
	std::vector<UINT>	m_LycWaitTime;//下段歌词的等待时间
	int		m_LycIndex;//当前歌词的行数
	wchar_t* m_TitleName;
	CPanel* m_background;//Title画面
	CPanel* m_MTVpreview;//MTV预览
	CPanel* m_MTVbackground;//MTV画面
	CUIButton* m_MTVbutton[MTV_NUM_MAX];//MTV按钮
	CUIButton* m_button_pause;//暂停
	CUIButton* m_button_back;//返回
	CUIButton* m_button_exit;//退出
	CUIFont* m_MTVtitle;//MTV名
	CUIFont* m_MTVsub;//字幕
	CUIFont* m_MTVsubFlow; // 滚动字幕
	unsigned int m_striptOriginalWidth;
	CPanel* m_MTVsubBG;//字幕背景
	std::map<CString ,std::vector<LPDIRECT3DTEXTURE9>> m_TexMap;
	void saveTimeEvent(); // method to execute tick saving.
	int findLetterIndex(float x);	// find which letter is clicked given the x position.

	FILE* m_SubFlowControlfile; //保存点击之后的字位置以及唱到该字的时间的文件
	bool m_isPressing; // flag to see if the ESC key is being pressed.
	std::map<int, std::map<int, UINT32>> m_LycWordmap; //歌词的字位置和时间的map，数组标签顺序【行数】【位置】【时间】
	void decodeDataToLyricUnit(CString str);	// 解析歌词数据
	int m_accurateLyricsIndex;	// 后一个字的位置
	int m_previousAccurateLyricsIndex;	// 
	int m_accurateLyricsCount;	// 每行歌词字的
	UINT32 m_nextPosition;	// 后一个字的时间
	UINT32 m_previousPosition;	// 前一个字的时间
	
	int m_PreLycIndex;

	void updateLyricsProcess();
};
#endif