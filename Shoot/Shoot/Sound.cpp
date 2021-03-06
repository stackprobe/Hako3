#include "Stdinc.h"

int LoadSound(autoList<uchar> *image)
{
	int h = LoadSoundMemByMemImage(image->ElementAt(0), image->GetCount());

	errorCase(h == -1);
	return h;
}
int DuplSound(int h)
{
	int dupl_h = DuplicateSoundMem(h);

	errorCase(dupl_h == -1);
	return dupl_h;
}
void UnloadSound(int h)
{
	if(DeleteSoundMem(h) != 0)
	{
		error();
	}
}

void SoundPlay(int h, int endless_mode, int from_top)
{
	switch(CheckSoundMem(h))
	{
	case 1: // ? 再生中
		return;

	case 0: // ? 再生していない。
		break;

	case -1: // ? エラー
	default:
		error();
	}
	if(PlaySoundMem(h, endless_mode ? DX_PLAYTYPE_LOOP : DX_PLAYTYPE_BACK, from_top) != 0) // ? 失敗
	{
		error();
	}
}
void SetVolume(int h, double volume)
{
	m_range(volume, 0.0, 1.0);

	int pal = m_d2i(volume * 255.0);

	errorCase(pal < 0 || 255 < pal);

	if(ChangeVolumeSoundMem(pal, h) != 0) // ? 失敗
	{
		error();
	}
}
void SoundStop(int h)
{
	if(StopSoundMem(h) != 0) // ? 失敗
	{
		error();
	}
}
