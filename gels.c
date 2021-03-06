#include "exec/types.h"
#include "exec/exec.h"
#include "graphics/gfx.h"
#include "graphics/gfxmacros.h"
#include "graphics/gfxbase.h"

#include "graphics/view.h"
#include "graphics/rastport.h"
#include "graphics/gels.h"
#include "graphics/collide.h"

#include "gels.h"

struct GelsInfo GelsInfo;
struct VSprite Start, End;
WORD *LastColors[8] = {0,0,0,0,0,0,0,0};
WORD NextLines[8] = {0,0,0,0,0,0,0,0};
struct collTable collTable;

VOID CreateGELS(struct RastPort * rastPort, int width, int height)
{
	BltClear(&Start, sizeof(struct VSprite), 0);
	BltClear(&End, sizeof(struct VSprite), 0);
	BltClear(&GelsInfo, sizeof(struct GelsInfo), 0);
	BltClear(&collTable, sizeof(struct collTable), 0);
	
	GelsInfo.sprRsrvd = 0x80;
	GelsInfo.nextLine = NextLines;
	GelsInfo.lastColor = LastColors;
	GelsInfo.collHandler = &collTable;
	GelsInfo.leftmost = 0;
	GelsInfo.rightmost = width-1;
	GelsInfo.topmost = 0;
	GelsInfo.bottommost = height-1;
	
	InitGels(&Start, &End, &GelsInfo);
	rastPort->GelsInfo = &GelsInfo;

	SetCollision(0, BorderController, &GelsInfo);
}

VOID SetGelsInfo(struct RastPort * RastPort)
{
	RastPort->GelsInfo = &GelsInfo;
}

VOID CreateBob(struct VSprite *vsbob, struct Bob *bob, int x , int y, IMAGE *image)
{
	BltClear(vsbob, sizeof(struct VSprite), 0);
	BltClear(bob, sizeof(struct Bob), 0);
	bob->SaveBuffer = 0;
	bob->DBuffer = 0;
	vsbob->CollMask = 0;
	vsbob->BorderLine = 0;
	
	vsbob->Width = image->Width / 16;
	vsbob->Height = image->Height;
	vsbob->Depth = image->Depth;
	vsbob->ImageData = image->ImageData;
	vsbob->X = x;
	vsbob->Y = y;
	vsbob->PlaneOnOff = 0x0;
	vsbob->PlanePick = 0xff;
	vsbob->Flags = OVERLAY;
	vsbob->MeMask = 0x0;
	vsbob->HitMask = 0x0;
	
	bob->Flags = 0;
	bob->BobVSprite = vsbob;
	vsbob->VSBob = bob;
}

VOID SetCollisionBob(struct VSprite *vsbob, struct Bob *bob, IMAGE *image)
{
	WORD *Borderline = (WORD*)AllocMem(sizeof(WORD) * (image->Width/16), MEMF_CLEAR|MEMF_CHIP);
	WORD *CollisionMask = (WORD*)AllocMem(RASSIZE(image->Width, image->Height), MEMF_CLEAR|MEMF_CHIP);

	vsbob->CollMask = CollisionMask;
	vsbob->BorderLine = Borderline; 
	
	bob->ImageShadow = CollisionMask;
	
	InitMasks(vsbob);
}
VOID SetDoubleBufferBob(struct VSprite *vsbob, struct Bob *bob, IMAGE *image)
{
	WORD *SaveBuffer = (WORD *)AllocMem(RASSIZE(image->Width, image->Height)*(image->Depth), MEMF_CLEAR|MEMF_CHIP);
	WORD *DBufBuffer = (WORD *)AllocMem(RASSIZE(image->Width, image->Height)*(image->Depth), MEMF_CLEAR|MEMF_CHIP);
	struct DBufPacket *DBufPackets  = (struct DBufPacket *)AllocMem(sizeof(struct DBufPacket), MEMF_CLEAR|MEMF_CHIP);
	
	vsbob->Flags |= SAVEBACK;
	
	bob->SaveBuffer = SaveBuffer;
	bob->DBuffer = DBufPackets;
	DBufPackets->BufBuffer = DBufBuffer;
}
VOID DeleteBob(struct VSprite *vsbob, struct Bob *bob)
{
	/*if(vsbob->ImageData !=0)
	{
		FreeMem(vsbob->ImageData, RASSIZE(vsbob->Width*16, vsbob->Height)*vsbob->Depth);
	}*/
	
	if(vsbob->CollMask !=0)
	{
		FreeMem(vsbob->CollMask, RASSIZE(vsbob->Width *16, vsbob->Height));
	}
	
	if(vsbob->BorderLine != 0)
	{
		FreeMem(vsbob->BorderLine, vsbob->Width * sizeof(UWORD));
	}
	
	
	if(bob->SaveBuffer !=0)
	{
		FreeMem(bob->SaveBuffer, RASSIZE(vsbob->Width *16, vsbob->Height)*(vsbob->Depth));
	}

	if(bob->DBuffer != 0)
	{
		if(bob->DBuffer->BufBuffer != 0)
		{
			FreeMem(bob->DBuffer->BufBuffer, RASSIZE(vsbob->Width *16, vsbob->Height)*(vsbob->Depth));
		}
		FreeMem(bob->DBuffer, sizeof(struct DBufPacket));
	}
	
}

VOID CreateVSprite(struct VSprite *vspr, int x, int y, IMAGE *image, UWORD *colours)
{
	BltClear(vspr, sizeof(struct VSprite), 0);
	vspr->CollMask = 0;
	vspr->BorderLine = 0;
	
	vspr->Width = 1;
	vspr->Height = image->Height;
	vspr->Flags = VSPRITE;
	vspr->Depth = 2;
	vspr->ImageData = image->ImageData;
	vspr->SprColors = colours;
	vspr->X = x;
	vspr->Y = y;
	
	vspr->MeMask = 0x0;
	vspr->HitMask = 0x0;
}
VOID SetCollisionVSprite(struct VSprite *vspr, IMAGE *image)
{
	WORD *Borderline = (WORD*)AllocMem(sizeof(WORD) * (image->Width/16), MEMF_CLEAR|MEMF_CHIP);
	WORD *CollisionMask = (WORD*)AllocMem(RASSIZE(image->Width, image->Height), MEMF_CLEAR|MEMF_CHIP);

	vspr->CollMask = CollisionMask;
	vspr->BorderLine = Borderline;
	
	InitMasks(vspr);
}
VOID DeleteVSprite(struct VSprite *vspr)
{
	if(vspr->ImageData !=0)
	{
		FreeMem(vspr->ImageData, RASSIZE(vspr->Width*16, vspr->Height)*vspr->Depth);
	}
	
	if(vspr->CollMask !=0)
	{
		FreeMem(vspr->CollMask, RASSIZE(vspr->Width *16, vspr->Height));
	}
	
	if(vspr->BorderLine != 0)
	{
		FreeMem(vspr->BorderLine, vspr->Width * sizeof(UWORD));
	}
}
VOID BorderController(struct VSprite vsprite, BYTE Border)
{
	/*do nothing at the moment*/
}