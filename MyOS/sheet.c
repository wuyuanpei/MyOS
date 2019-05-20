/* Sheet Controller: store all the sheets on the screen */

#include "bootpack.h"

/* Initialize Sheet Controller */
void shtctl_init(unsigned char *vram, int xsize, int ysize)
{
	struct SHTCTL *ctl = (struct SHTCTL *)ADR_SHTCTL;
	int i;
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1; /* No sheet on the screen */
	for (i = 0; i < MAX_SHEETS; i++)
		ctl->sheets0[i].flags = SHEET_UNUSED;
}

/* Get an unused sheet */
struct SHEET *sheet_alloc(void)
{
	struct SHTCTL *ctl = (struct SHTCTL *)ADR_SHTCTL;
	struct SHEET *sht;
	int i;
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets0[i].flags == SHEET_UNUSED) {
			sht = & ctl->sheets0[i];
			sht->flags = SHEET_USED;
			/* Conceal the sheet: when height is -1, the sheet won't be displayed */
			sht->height = -1;
			return sht;
		}
	}
	return 0;	/* No empty sheet left */
}

/* Set a sheet */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

/* Refresh the screen only from [vx0,vx1), [vy0,vy1) in absolute coordinate */
void sheet_refreshsub(int vx0, int vy0, int vx1, int vy1)
{
	struct SHTCTL *ctl = (struct SHTCTL *)ADR_SHTCTL;
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, c, *vram = ctl->vram;
	struct SHEET *sht;
	/* Only refresh the part on the screen */
	if(vx0 < 0) vx0 = 0;
	if(vy0 < 0) vy0 = 0;
	if(vx1 > ctl->xsize) vx1 = ctl->xsize;
	if(vy1 > ctl->ysize) vy1 = ctl->ysize;
	/* Iterate every sheet */
	for (h = 0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		/* Calculate relative coordinate in this sheet from absolute coordinates */
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		/* Adjust the boundary (only refresh the screen in the region specified) */
		if (bx0 < 0) { bx0 = 0; }
		if (by0 < 0) { by0 = 0; }
		if (bx1 > sht->bxsize) { bx1 = sht->bxsize; }
		if (by1 > sht->bysize) { by1 = sht->bysize; }
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				c = buf[by * sht->bxsize + bx];
				if (c != sht->col_inv) {
					vram[vy * ctl->xsize + vx] = c;
				}
			}
		}
	}
}

/* Set height for a sheet */
void sheet_updown(struct SHEET *sht, int height)
{
	struct SHTCTL *ctl = (struct SHTCTL *)ADR_SHTCTL;
	int h, old = sht->height; /* store the original height */

	/* Adjust the beyond border values */
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	if (height < -1) {
		height = -1;
	}
	sht->height = height; /* Set the height */

	/* Rearrage sheets[] */
	if (old > height) {	/* height become smaller */
		/* Displayable */
		if (height >= 0) {
			/* Adjust the sheets in between */
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			/* Move the sheet */
			ctl->sheets[height] = sht;
		} else {	/* Not displayable */
			/* If it is not the heighest one */
			if (ctl->top > old) {
				/* Adjust the heigher sheets */
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--; /* The number of displayable sheets decrement */
		}
		sheet_refreshsub(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
	} else if (old < height) {	/* height become greater */
		/* The original height is displayable */
		if (old >= 0) {
			/* Adjust the sheets in between */
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			/* Move the sheet */
			ctl->sheets[height] = sht;
		} else {	/* Not displayable becomes displayable */
			/* Put the heigher ones one position up (ATTENTION: new height must be [0,top+1]) */
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++; /* One more displayable sheet */
		}
		sheet_refreshsub(sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize);
	}
}

/* Refresh a sheet only in the relative region specified */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1)
{
	if (sht->height >= 0) { /* Displayable */
		sheet_refreshsub(sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1);
	}
}

/* Move the sheet (reset its absolute address) */
void sheet_slide(struct SHEET *sht, int vx0, int vy0)
{
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	if (sht->height >= 0) { /* Refresh the original region and the updated region */
		sheet_refreshsub(old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize);
		sheet_refreshsub(vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize);
	}
}

/* mark a sheet unused */
void sheet_free(struct SHEET *sht)
{
	if (sht->height >= 0) {
		sheet_updown(sht, -1);
	}
	sht->flags = SHEET_UNUSED;
}