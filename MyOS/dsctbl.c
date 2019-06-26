/* Set descriptor table */
#include "bootpack.h"
#include <stdio.h>

static int application_exception(int *esp, char *exception);

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
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, AR_DATA32_RW); // All memory for OS data
	set_segmdesc(gdt + 2, LIMIT_BOTPAK, ADR_BOTPAK, AR_CODE32_ER); // For bootpack (OS code)
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
	set_gatedesc(idt + 0x00, (int) asm_inthandler00, 2 << 3, AR_INTGATE32);
	set_gatedesc(idt + 0x06, (int) asm_inthandler06, 2 << 3, AR_INTGATE32);
	set_gatedesc(idt + 0x0c, (int) asm_inthandler0c, 2 << 3, AR_INTGATE32);
	set_gatedesc(idt + 0x0d, (int) asm_inthandler0d, 2 << 3, AR_INTGATE32);
	set_gatedesc(idt + 0x20, (int) asm_inthandler20, 2 << 3, AR_INTGATE32);
	set_gatedesc(idt + 0x21, (int) asm_inthandler21, 2 << 3, AR_INTGATE32);
	set_gatedesc(idt + 0x2c, (int) asm_inthandler2c, 2 << 3, AR_INTGATE32);
	// API
	set_gatedesc(idt + 0x30, (int) api_call, 2 << 3, AR_INTGATE32 + 0x60); // The only INT that can be called by application
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

// Dealing with divide by zero exception
int inthandler00(int *esp)
{
	return application_exception(esp, "INT 00: Divide By Zero Exception");
}

// Dealing with illegal instruction exception
int inthandler06(int *esp)
{
	return application_exception(esp, "INT 06: Illegal Instruction Exception");
}

// Dealing with protected exception
int inthandler0d(int *esp)
{
	return application_exception(esp, "INT 0d: General Protected Exception");
}

// Dealing with stack exception
int inthandler0c(int *esp)
{
	return application_exception(esp, "INT 0c: Stack Exception");
}

// Print out the register values
static int application_exception(int *esp, char *exception)
{
	char buf[50];
	print_string(exception);
	sprintf(buf,"EAX 0x%08x; EBX 0x%08x; ECX 0x%08x",esp[7],esp[4],esp[6]);
	print_string(buf);
	sprintf(buf,"EDX 0x%08x; EDI 0x%08x; ESI 0x%08x",esp[5],esp[0],esp[1]);
	print_string(buf);
	sprintf(buf,"ESP 0x%08x; EBP 0x%08x; EIP 0x%08x",esp[14],esp[2],esp[11]);
	print_string(buf);
	return &(task_now()->tss.esp0); // When return tss.esp0, kill the application
}



