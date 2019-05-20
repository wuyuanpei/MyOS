/* Set descriptor table */
#include "bootpack.h"

void init_gdtidt(void)
{
	/* Memory location for gdt and idt */
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct GATE_DESCRIPTOR    *idt = (struct GATE_DESCRIPTOR    *) ADR_IDT;
	int i;
	int gdt_entry_number = LIMIT_GDT >> 3; // 8192 entries
	int idt_entry_number = LIMIT_IDT >> 3; // 256 entries
	/* Initialize GDT */
	for (i = 0; i < gdt_entry_number; i++) { // 8192 entries
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW); // All memory
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER); // For bootpack
	load_gdtr(LIMIT_GDT, ADR_GDT); // Set GDTR register

	/* Initialize IDT */
	for (i = 0; i < idt_entry_number; i++) { // 256 entries
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(LIMIT_IDT, ADR_IDT); // Set IDTR register
	
	/* Set IDT entries*/
	// set_gatedesc(entry location in IDT, function pointer, 
	// the segment number of the function << 3, access right);
	// AR_INTGATE32 (0x8e): the interrupt is effective
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 << 3, AR_INTGATE32);
	//set_gatedesc(idt + 0x27, (int) asm_inthandler27, 2 << 3, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 << 3, AR_INTGATE32);
}

void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit,
 int base, int ar)
{
	if (limit > 0xfffff) {
		ar |= 0x8000; // G_bit = 1
		limit >>= 12; // limit /= 12
	}
	sd->limit_low    = limit & 0xffff;
	sd->base_low     = base & 0xffff;
	sd->base_mid     = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high   = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high    = (base >> 24) & 0xff;
}

void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset,
 int selector, int ar)
{
	gd->offset_low   = offset & 0xffff;
	gd->selector     = selector;
	gd->dw_count     = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high  = (offset >> 16) & 0xffff;
}



