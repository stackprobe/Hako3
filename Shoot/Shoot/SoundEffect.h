enum
{
	SE_000,
	SE_001,
	SE_002,
	SE_003,
	SE_004,
	SE_005,
	SE_006,
	SE_007,
	SE_008,
	SE_009,
	SE_010,
	SE_011,
	SE_012,
	SE_013,
	SE_014,
	SE_015,
	SE_016,
	SE_017,
	SE_018,
	SE_019,
	SE_020,
	SE_021,
	SE_022,
	SE_023,
	SE_024,
	SE_025,
	SE_026,
	SE_027,
	SE_028,
	SE_029,
	SE_030,
	SE_031,
	SE_032,
	SE_033,
	SE_034,
	SE_035,
	SE_036,
	SE_037,
	SE_038,
	SE_039,
	SE_040,
	SE_041,
	SE_042,
	SE_043,
	SE_044,
	SE_045,
	SE_046,
	SE_047,
	SE_048,
	SE_049,
	SE_050,
	SE_051,
	SE_052,
	SE_053,
	SE_054,
	SE_055,
	SE_056,
	SE_057,
	SE_058,
	SE_059,
	SE_060,
	SE_061,
	SE_062,
	SE_063,
	SE_064,
	SE_065,
	SE_066,
	SE_067,
	SE_068,
	SE_069,
	SE_070,
	SE_071,
	SE_072,
	SE_073,
	SE_074,
	SE_075,
	SE_076,
	SE_077,
	SE_078,
	SE_079,
	SE_080,
	SE_081,
	SE_082,
	SE_083,
	SE_084,
	SE_085,
	SE_086,
	SE_087,
	SE_088,
	SE_089,
	SE_090,
	SE_091,
	SE_092,
	SE_093,
	SE_094,
	SE_095,
	SE_096,
	SE_097,
	SE_098,
	SE_099,
	SE_100,
	SE_101,
	SE_102,
	SE_103,
	SE_104,
	SE_105,
	SE_106,
	SE_107,
	SE_108,
	SE_109,
	SE_110,
	SE_111,

	SE_MAX, // num of member
};

singleton_proto_VT(SELoadedList, autoList<int>)
singleton_proto_VT(SEVolumeList, autoList<double>)

int SE_Load(int resno, autoList<uchar> *image);
void SE_Unload(int resno, int h);
Resource_t *SE_Res(void);

void ReleaseSoundEffects(void);
int GetSERequestCount(void);

int SEFrame(void);

void SETouch(int resno);
void SEPlay(int resno);
void SEPlayEcho(int resno);
void SEStopEcho(void);
void UpdateSEVolume(void);
