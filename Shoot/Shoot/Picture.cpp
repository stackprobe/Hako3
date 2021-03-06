#include "Stdinc.h"

static singleton_VTI(RGBAStack, autoList<int>, new autoList<int>())

void EnterRGBA(void)
{
	S_RGBAStack()->AddElement(Color_R);
	S_RGBAStack()->AddElement(Color_G);
	S_RGBAStack()->AddElement(Color_B);
	S_RGBAStack()->AddElement(Color_A);
}
void LeaveRGBA(void)
{
	Color_A = S_RGBAStack()->UnaddElement();
	Color_B = S_RGBAStack()->UnaddElement();
	Color_G = S_RGBAStack()->UnaddElement();
	Color_R = S_RGBAStack()->UnaddElement();
}

void SILoadPixel(int h, int x, int y)
{
	if(GetPixelSoftImage(h, x, y, &Color_R, &Color_G, &Color_B, &Color_A) != 0) // ? 失敗
	{
		error();
	}
	ec_range(Color_R, 0, 255);
	ec_range(Color_G, 0, 255);
	ec_range(Color_B, 0, 255);
	ec_range(Color_A, 0, 255);
}
void SISavePixel(int h, int x, int y)
{
	m_range(Color_R, 0, 255);
	m_range(Color_G, 0, 255);
	m_range(Color_B, 0, 255);
	m_range(Color_A, 0, 255);

	if(DrawPixelSoftImage(h, x, y, Color_R, Color_G, Color_B, Color_A) != 0) // ? 失敗
	{
		error();
	}
}

int PL_Resno = HANDLE_CLOSED;

int PL_ImageIsBmp(autoList<uchar> *image)
{
	return 2 <= image->GetCount() && !memcmp(image->ElementAt(0), "BM", 2);
}
int Pic_Load(int resno, autoList<uchar> *image)
{
	void (*si_filter1)(int, int, int) = S_SoftImageFilterList()->GetElement(resno);
	void (*si_filter2)(int, int, int) = GetPictureGroup()->SIFilter;
	void (*gh_filter1)(int) = S_GraphicHandleFilterList()->GetElement(resno);
	void (*gh_filter2)(int) = GetPictureGroup()->GHFilter;
	int h;

//	LOG("[LoadPic] resno: %d\n", resno);

	PL_Resno = resno;

	if(si_filter1 || si_filter2 || PL_ImageIsBmp(image))
	{
//		LOG("[LoadPic] SoftImage経由\n");

		h = LoadSoftImageToMem(image->ElementAt(0), image->GetCount());
		errorCase(h == -1); // ? 失敗

		int x_size;
		int y_size;

		if(GetSoftImageSize(h, &x_size, &y_size) != 0) // ? 失敗
		{
			error();
		}
		errorCase(x_size < 1 || IMAX < x_size);
		errorCase(y_size < 1 || IMAX < y_size);

		// RGB etc. -> RGBA
		{
			int h2 = MakeARGB8ColorSoftImage(x_size, y_size);
			errorCase(h2 == -1); // ? 失敗

			errorCase(BltSoftImage(0, 0, x_size, y_size, h, 0, 0, h2)); // ? 失敗
			errorCase(DeleteSoftImage(h) == -1);

			h = h2;
		}

		EnterRGBA();

		if(PL_ImageIsBmp(image))
		{
//			LOG("[LoadPic] 黒の透明化\n");
			SIF_BlackToTrans(h, x_size, y_size);
		}
		if(si_filter1) si_filter1(h, x_size, y_size);
		if(si_filter2) si_filter2(h, x_size, y_size);

		LeaveRGBA();

		int grph_h = CreateGraphFromSoftImage(h);
		errorCase(grph_h == -1); // ? 失敗

		if(DeleteSoftImage(h) == -1) // ? 失敗
		{
			error();
		}
		h = grph_h;
	}
	else
	{
//		LOG("[LoadPic] Direct\n");

		h = CreateGraphFromMem(image->ElementAt(0), image->GetCount());
		if(h == -1)
		{
			printfDx("%d\n", resno);
			error();
		}
//		errorCase(h == -1); // ? 失敗
	}

	if(gh_filter1) gh_filter1(h);
	if(gh_filter2) gh_filter2(h);

	return h;
}
void Pic_Unload(int resno, int h)
{
	ReleaseDer(resno);

	if(DeleteGraph(h) != 0)
	{
		error();
	}
}
static Resource_t *Pic_Res_pg(PictureGroup_t *pg)
{
	if(!pg->Res)
		pg->Res = CreateResource("Picture.rclu", "..\\..\\Picture.txt", P_MAX, Pic_Load, Pic_Unload);

	return pg->Res;
}
Resource_t *Pic_Res(void)
{
	return Pic_Res_pg(GetPictureGroup());
}

void ReleasePictures(void)
{
	autoList<PictureGroup_t *> *pgl = S_PictureGroupList();

	for(int index = 0; index < pgl->GetCount(); index++)
	{
		UnloadAllHandle(Pic_Res_pg(pgl->GetElement(index)));
	}
}

singleton_VTI(SoftImageFilterList,     autoList<void (*)(int, int, int)>, makeList(P_MAX, (void (*)(int, int, int))NULL))
singleton_VTI(GraphicHandleFilterList, autoList<void (*)(int)>,           makeList(P_MAX, (void (*)(int))NULL))
singleton_VTI(EnableTransList,         autoList<int>,                     makeList(P_MAX, 1))

int EnableTrans;

int Pic(int resno)
{
	if(resno & DTP)
	{
		return Der(resno & ~DTP);
	}
	int h = GetHandle(Pic_Res(), resno);

	/*
		ここで設定された値を次の描画時に参照する。
		ので DrawPic(x, y, Pic(P_*)) のように、描画系関数の引数に直接 Pic() を記述して、
		Pic() の呼び出しと描画の間に別の Pic() が入る余地の無い記述を心掛けること。
	*/
	EnableTrans = S_EnableTransList()->GetElement(resno);
	return h;
}

static void SetBlend(int mode, double a)
{
	m_range(a, 0.0, 1.0);

	int pal = m_d2i(a * 255.0);

	errorCase(pal < 0 || 255 < pal);

	if(SetDrawBlendMode(mode, pal) != 0)
	{
		error();
	}
}
void SetAlpha(double a) // a: 0.0 - 1.0 == 透明 - 不透明
{
	SetBlend(DX_BLENDMODE_ALPHA, a);
}
void SetBlendAdd(double a) // a: 0.0 - 1.0 == 描画しない - 真っ白?
{
	SetBlend(DX_BLENDMODE_ADD, a); // けっこう重い!?
}
void SetBlendInv(double a) // a: 不使用?
{
	SetBlend(DX_BLENDMODE_INVSRC, a);
}
void ResetBlend(void)
{
	if(SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0) != 0)
	{
		error();
	}
}
void SetBright(double r, double g, double b) // (r, g, b): 0.0 - 1.0 == 黒 - 色彩あり
{
	m_range(r, 0.0, 1.0);
	m_range(g, 0.0, 1.0);
	m_range(b, 0.0, 1.0);

	int pal_r = m_d2i(r * 255.0);
	int pal_g = m_d2i(g * 255.0);
	int pal_b = m_d2i(b * 255.0);

	errorCase(pal_r < 0 || 255 < pal_r);
	errorCase(pal_g < 0 || 255 < pal_g);
	errorCase(pal_b < 0 || 255 < pal_b);

	if(SetDrawBright(pal_r, pal_g, pal_b) != 0)
	{
		error();
	}
}
void ResetBright(void)
{
	if(SetDrawBright(255, 255, 255) != 0)
	{
		error();
	}
}

int Pic_W;
int Pic_H;

void CheckPicSize(int h)
{
	Pic_W = -1;
	Pic_H = -1;

	GetGraphSize(h, &Pic_W, &Pic_H);

	errorCase(Pic_W < 1 || IMAX < Pic_W);
	errorCase(Pic_H < 1 || IMAX < Pic_H);
}

int ScreenSlip_X;
int ScreenSlip_Y;

void SimpleDrawPic(int x, int y, int h)
{
	DrawGraph(x + ScreenSlip_X, y + ScreenSlip_Y, h, EnableTrans);
}
void SimpleDrawPic(double x, double y, int h)
{
	SetDrawMode(DX_DRAWMODE_BILINEAR); // ジャギー対策

	DrawGraphF(
		(float)(x + ScreenSlip_X),
		(float)(y + ScreenSlip_Y),
		h,
		EnableTrans
		);

	SetDrawMode(DX_DRAWMODE_NEAREST); // デフォに戻す
}
void SimpleDrawPicFloat(double x, double y, int h)
{
	SetDrawMode(DX_DRAWMODE_BILINEAR); // ジャギー対策

#if 0
	CheckPicSize(h);
	DrawModiGraphF(
		(float)x,
		(float)y,
		(float)(x + Pic_W),
		(float)y,
		(float)(x + Pic_W),
		(float)(y + Pic_H),
		(float)x,
		(float)(y + Pic_H),
		h,
		EnableTrans
		);
#else
	DrawGraphF((float)x, (float)y, h, EnableTrans);
#endif

	SetDrawMode(DX_DRAWMODE_NEAREST); // デフォに戻す
}
void DrawPic(int x, int y, int h)
{
	CheckPicSize(h);
	SimpleDrawPic(x - Pic_W / 2, y - Pic_H / 2, h);
}
void DrawPic(double x, double y, int h)
{
	CheckPicSize(h);
	SimpleDrawPic(x - Pic_W / 2.0, y - Pic_H / 2.0, h);
}
void DrawPicRect(int x, int y, int w, int h, int hdl)
{
	x += ScreenSlip_X;
	y += ScreenSlip_Y;

	errorCase(x < -IMAX || IMAX < x);
	errorCase(y < -IMAX || IMAX < y);
	errorCase(w <     1 || IMAX < w);
	errorCase(h <     1 || IMAX < h);
	
	if(DrawModiGraph(x, y, x + w, y, x + w, y + h, x, y + h, hdl, EnableTrans) != 0)
	{
		error();
	}
}
void DrawPicFree(
	double ltx, double lty, // 左上
	double rtx, double rty, // 右上
	double rbx, double rby, // 右下
	double lbx, double lby, // 左下
	int hdl
	)
{
	if(ScreenSlip_X)
	{
		ltx += ScreenSlip_X;
		rtx += ScreenSlip_X;
		rbx += ScreenSlip_X;
		lbx += ScreenSlip_X;
	}
	if(ScreenSlip_Y)
	{
		lty += ScreenSlip_Y;
		rty += ScreenSlip_Y;
		rby += ScreenSlip_Y;
		lby += ScreenSlip_Y;
	}

	SetDrawMode(DX_DRAWMODE_BILINEAR); // ジャギー対策

	if(DrawModiGraphF(
		(float)ltx,
		(float)lty,
		(float)rtx,
		(float)rty,
		(float)rbx,
		(float)rby,
		(float)lbx,
		(float)lby,
		hdl,
		EnableTrans
		) != 0
		)
	{
		LOG("[ERROR-DrawPicRect] %f %f %f %f %f %f %f %f %d\n", ltx, lty, rtx, rty, rbx, rby, lbx, lby, hdl);
		//error();
	}
	SetDrawMode(DX_DRAWMODE_NEAREST); // デフォに戻す
}

static double Draw_X;
static double Draw_Y;
static double Draw_LT_X;
static double Draw_LT_Y;
static double Draw_RT_X;
static double Draw_RT_Y;
static double Draw_RB_X;
static double Draw_RB_Y;
static double Draw_LB_X;
static double Draw_LB_Y;
static int Draw_Handle;
static int Draw_EnableTrans;

void DrawBegin(double x, double y, int h)
{
	if(ScreenSlip_X)
		x += (double)ScreenSlip_X;

	if(ScreenSlip_Y)
		y += (double)ScreenSlip_Y;

	Draw_X = x;
	Draw_Y = y;

	CheckPicSize(h);

	double hw = Pic_W / 2.0;
	double hh = Pic_H / 2.0;

	Draw_LT_X = -hw;
	Draw_LT_Y = -hh;
	Draw_RT_X =  hw;
	Draw_RT_Y = -hh;
	Draw_RB_X =  hw;
	Draw_RB_Y =  hh;
	Draw_LB_X = -hw;
	Draw_LB_Y =  hh;

	Draw_Handle = h;
	Draw_EnableTrans = EnableTrans;
}
void DrawMove(double x, double y)
{
	Draw_LT_X += x;
	Draw_RT_X += x;
	Draw_RB_X += x;
	Draw_LB_X += x;

	Draw_LT_Y += y;
	Draw_RT_Y += y;
	Draw_RB_Y += y;
	Draw_LB_Y += y;
}
void DrawRotate(double rot) // rot: 時計回り, 0.0 == 0度, PI == 180度
{
	double w;

#define ROTATE(x, y) \
	w = x * cos(rot) - y * sin(rot); \
	y = x * sin(rot) + y * cos(rot); \
	x = w

	ROTATE(Draw_LT_X, Draw_LT_Y);
	ROTATE(Draw_RT_X, Draw_RT_Y);
	ROTATE(Draw_RB_X, Draw_RB_Y);
	ROTATE(Draw_LB_X, Draw_LB_Y);
}
void DrawRotate(int numer, int denom)
{
	errorCase(numer < -IMAX || IMAX < numer);
	errorCase(denom <     1 || IMAX < denom);

	numer %= denom;

	DrawRotate((PI * 2.0) * ((double)numer / denom));
}
void DrawXZoom(double zoom)
{
	Draw_LT_X *= zoom;
	Draw_RT_X *= zoom;
	Draw_RB_X *= zoom;
	Draw_LB_X *= zoom;
}
void DrawYZoom(double zoom)
{
	Draw_LT_Y *= zoom;
	Draw_RT_Y *= zoom;
	Draw_RB_Y *= zoom;
	Draw_LB_Y *= zoom;
}
void DrawZoom(double zoom)
{
	DrawXZoom(zoom);
	DrawYZoom(zoom);
}
void DrawEnd(void)
{
	SetDrawMode(DX_DRAWMODE_BILINEAR); // ジャギー対策

	if(
#if 1
		DrawModiGraphF(
			(float)(Draw_X + Draw_LT_X),
			(float)(Draw_Y + Draw_LT_Y),
			(float)(Draw_X + Draw_RT_X),
			(float)(Draw_Y + Draw_RT_Y),
			(float)(Draw_X + Draw_RB_X),
			(float)(Draw_Y + Draw_RB_Y),
			(float)(Draw_X + Draw_LB_X),
			(float)(Draw_Y + Draw_LB_Y),
			Draw_Handle,
			Draw_EnableTrans) != 0
#else
		DrawModiGraph(
			(int)(Draw_X + Draw_LT_X),
			(int)(Draw_Y + Draw_LT_Y),
			(int)(Draw_X + Draw_RT_X),
			(int)(Draw_Y + Draw_RT_Y),
			(int)(Draw_X + Draw_RB_X),
			(int)(Draw_Y + Draw_RB_Y),
			(int)(Draw_X + Draw_LB_X),
			(int)(Draw_Y + Draw_LB_Y),
			Draw_Handle,
			Draw_EnableTrans) != 0
#endif
		)
	{
		printfDx("Draw_X:%f\n", Draw_X);
		printfDx("Draw_Y:%f\n", Draw_Y);
		printfDx("Draw_LT_X:%f\n", Draw_LT_X);
		printfDx("Draw_LT_Y:%f\n", Draw_LT_Y);
		printfDx("Draw_RT_X:%f\n", Draw_RT_X);
		printfDx("Draw_RT_Y:%f\n", Draw_RT_Y);
		printfDx("Draw_RB_X:%f\n", Draw_RB_X);
		printfDx("Draw_RB_Y:%f\n", Draw_RB_Y);
		printfDx("Draw_LB_X:%f\n", Draw_LB_X);
		printfDx("Draw_LB_Y:%f\n", Draw_LB_Y);
		printfDx("Draw_Handle:%d\n", Draw_Handle);
		printfDx("Draw_EnableTrans:%d\n", Draw_EnableTrans);

		error();
	}

	SetDrawMode(DX_DRAWMODE_NEAREST); // デフォに戻す
}
