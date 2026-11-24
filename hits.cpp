#include "global.h"

int curhit;
int *hitSelected;


int GetHitIndex(SetBrush *b)
{
    for (int i = 0; i < numhits; i++)
        if (besthits[i] == b)
            return i;
    return -1;
}

void ClearHits()
{
    memset(besthits,0,MAX_HITS*sizeof(SetBrush *));
    memset(hitSelected,0,MAX_HITS*sizeof(int));
    numhits = 0;
    curhit = 0;
}

void RemoveHitBrush(SetBrush *b)
{
    if (!numhits)
        return;
    if (!besthits)
        return;
    if (!hitSelected)
        return;

    for (int i = 0; i < numhits; i++)
    {
        if (besthits[i] == b)
        {
            numhits--;
            besthits[i] = besthits[numhits];
            hitSelected[i] = hitSelected[numhits];
            besthits[numhits] = NULL;
            hitSelected[numhits] = 0;
            break;
        }
    }
}

void AddHitBrush(SetBrush *b)
{
    if (numhits < MAX_HITS)
    {
        besthits[numhits] = b;
        hitSelected[numhits] = b->IsSelected(); // save original state...
        numhits++;
    }
}

void SelectHitBrush(int index)
{
    if (index >= 0 && index < numhits)
    {
        SetBrush *b = besthits[index];
        if (b)
            b->setSelected(!b->IsSelected());
    }
    curhit = index;
}

void SelectAllHit()
{
    for (int sel = 0; sel < numhits; sel++)
    {
        SetBrush *b = besthits[sel];
        if (b)
            b->setSelected(true);
    }
}

void HitRestore()
{
    for (int sel = 0; sel < numhits; sel++)
    {
        SetBrush *b = besthits[sel];
        if (b)
            b->setSelected(b->getSaveSelected());
    }
}

void NextHitBrush()
{
    if (curhit >= 0 && curhit < numhits)
    {
        SetBrush *b = besthits[curhit];
        if (b)
        {
            b->setSelected(0 != hitSelected[curhit]); // return to previous state...
        }
        curhit = (curhit+1)%numhits;
        b = besthits[curhit];
        if (b)
        {
            b->setSelected(true);
        }
    }
}

void PreviousHitBrush()
{
    if (curhit >= 0 && curhit < numhits)
    {
        SetBrush *b = besthits[curhit];
        if (b)
        {
            b->setSelected(0 != hitSelected[curhit]); // return to previous state...
        }
        curhit--;

        if (curhit < 0)
        {
            curhit = numhits-1;
        }

        b = besthits[curhit];
        if (b)
        {
            b->setSelected(true);
        }
    }
}
