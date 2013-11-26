//
// Copyright (c) 2013-2014 Samuel Villarreal
// svkaiser@gmail.com
// 
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
//    1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 
 //   2. Altered source versions must be plainly marked as such, and must not be
 //   misrepresented as being the original software.
// 
//    3. This notice may not be removed or altered from any source
//    distribution.
// 
//-----------------------------------------------------------------------------
//
// DESCRIPTION:
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "wad.h"
#include "mapData.h"

//
// kexDoomMap::kexDoomMap
//

kexDoomMap::kexDoomMap(void) {
    this->mapLines          = NULL;
    this->mapVerts          = NULL;
    this->mapSides          = NULL;
    this->mapSectors        = NULL;
    this->mapSegs           = NULL;
    this->mapSSects         = NULL;
    this->nodes             = NULL;
    this->leafs             = NULL;
    this->ssLeafLookup      = NULL;
    this->ssLeafCount       = NULL;
    this->segSurfaces[0]    = NULL;
    this->segSurfaces[1]    = NULL;
    this->segSurfaces[2]    = NULL;
    this->leafSurfaces[0]   = NULL;
    this->leafSurfaces[1]   = NULL;

    this->numLeafs      = 0;
    this->numLines      = 0;
    this->numVerts      = 0;
    this->numSides      = 0;
    this->numSectors    = 0;
    this->numSegs       = 0;
    this->numSSects     = 0;
    this->numNodes      = 0;
}

//
// kexDoomMap::~kexDoomMap
//

kexDoomMap::~kexDoomMap(void) {
    Mem_Free(this->leafs);
    Mem_Free(this->ssLeafLookup);
    Mem_Free(this->ssLeafCount);

    this->leafs         = NULL;
    this->ssLeafLookup  = NULL;
    this->ssLeafCount   = NULL;
}

//
// kexDoomMap::BuildMapFromWad
//

void kexDoomMap::BuildMapFromWad(kexWadFile &wadFile) {
    lump_t          *lump;

    lump = wadFile.GetLumpFromName("VERTEXES");
    mapVerts = (mapVertex_t*)wadFile.GetLumpData(lump);
    numVerts = lump->size / sizeof(mapVertex_t);

    lump = wadFile.GetLumpFromName("LINEDEFS");
    mapLines = (mapLineDef_t*)wadFile.GetLumpData(lump);
    numLines = lump->size / sizeof(mapLineDef_t);

    lump = wadFile.GetLumpFromName("SIDEDEFS");
    mapSides = (mapSideDef_t*)wadFile.GetLumpData(lump);
    numSides = lump->size / sizeof(mapSideDef_t);

    lump = wadFile.GetLumpFromName("SECTORS");
    mapSectors = (mapSector_t*)wadFile.GetLumpData(lump);
    numSectors = lump->size / sizeof(mapSector_t);

    lump = wadFile.GetLumpFromName("SEGS");
    mapSegs = (mapSeg_t*)wadFile.GetLumpData(lump);
    numSegs = lump->size / sizeof(mapSeg_t);

    lump = wadFile.GetLumpFromName("SSECTORS");
    mapSSects = (mapSubSector_t*)wadFile.GetLumpData(lump);
    numSSects = lump->size / sizeof(mapSubSector_t);

    lump = wadFile.GetLumpFromName("NODES");
    nodes = (mapNode_t*)wadFile.GetLumpData(lump);
    numNodes = lump->size / sizeof(mapNode_t);

    BuildLeafs(wadFile);
}

//
// kexDoomMap::BuildLeafs
//

void kexDoomMap::BuildLeafs(kexWadFile &wadFile) {
    lump_t  *lump;
    int     i;
    int     j;
    short   *mlf;
    int     length;
    int     size;
    int     count;

    lump = wadFile.GetLumpFromName("LEAFS");

    if(lump == NULL) {
        return;
    }

    length = lump->size;
    mlf = (short*)wadFile.GetLumpData(lump);

    count = 0;
    size = 0;

    if(length) {
        short   *src = mlf;
        int     next;

        while(((byte*)src - (byte*)mlf) < length) {
            count++;
            size += (word)*src;
            next = (*src << 2) + 2;
            src += (next >> 1);
        }
    }

    if(count != numSSects) {
        Error("kexDoomMap::BuildLeafs: leaf/subsector inconsistancy %d/%d\n",
            count, numSSects);
    }

    if(count <= 0) {
        return;
    }

    leafs = (leaf_t*)Mem_Calloc((size * 2) * sizeof(leaf_t), hb_static);
    numLeafs = numSSects;

    ssLeafLookup = (int*)Mem_Calloc(sizeof(int) * numSSects, hb_static);
    ssLeafCount = (int*)Mem_Calloc(sizeof(int) * numSSects, hb_static);

    count = 0;

    int lfNum = 0;

    for(i = 0; i < numLeafs; i++) {
        ssLeafCount[i] = (word)*mlf++;
        ssLeafLookup[i] = lfNum;

        if(ssLeafCount[i]) {
            int vertex;
            int seg;
            
            for(j = 0; j < ssLeafCount[i]; j++, lfNum++) {
                vertex = (word)*mlf++;
                if(vertex > numVerts) {
                    Error("kexDoomMap::BuildLeafs: vertex out of range: %i - %i\n",
                        vertex, numVerts);
                }
                
                leafs[lfNum].vertex = &mapVerts[vertex];
                
                seg = *mlf++;
                if(seg == -1) {
                    leafs[lfNum].seg = NULL;
                }
                else {
                    if(seg > numSegs) {
                        Error("kexDoomMap::BuildLeafs: seg out of range: %i - %i\n",
                            seg, numSegs);
                    }
                    
                    leafs[lfNum].seg = &mapSegs[(word)seg];
                }
            }
        }
    }
}

//
// kexDoomMap::GetSideDef
//

mapSideDef_t *kexDoomMap::GetSideDef(const mapSeg_t *seg) {
    mapLineDef_t *line;

    if(seg->linedef == -1) {
        return NULL;
    }

    line = &mapLines[seg->linedef];
    return &mapSides[line->sidenum[seg->side]];
}

//
// kexDoomMap::GetFrontSector
//

mapSector_t *kexDoomMap::GetFrontSector(const mapSeg_t *seg) {
    mapSideDef_t *side = GetSideDef(seg);

    if(side == NULL) {
        return NULL;
    }

    return &mapSectors[side->sector];
}

//
// kexDoomMap::GetBackSector
//

mapSector_t *kexDoomMap::GetBackSector(const mapSeg_t *seg) {
    mapLineDef_t *line;

    if(seg->linedef == -1) {
        return NULL;
    }

    line = &mapLines[seg->linedef];

    if(line->flags & ML_TWOSIDED) {
        mapSideDef_t *backSide = &mapSides[line->sidenum[seg->side^1]];
        return &mapSectors[backSide->sector];
    }

    return NULL;
}