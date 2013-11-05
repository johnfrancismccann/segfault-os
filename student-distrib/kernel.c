/* kernel.c - the C part of the kernel
 * vim:ts=4 noexpandtab
 */

#include "multiboot.h"
#include "x86_desc.h"
#include "lib.h"
#include "i8259.h"
#include "debug.h"
#include "setup_idt.h"
#include "rtc.h"
#include "paging.h"
#include "keyboard.h"
#include "terminal.h"
#include "fs.h"
#include "tests.h"

/* Macros. */
/* Check if the bit BIT in FLAGS is set. */
#define CHECK_FLAG(flags,bit)   ((flags) & (1 << (bit)))


/* Test enables */
#define TEST_HANDIN_CODE 0      //Set to 1 to test the code Sam wrote during mp3.2 handin
#define TEST_TERM_READ 0		//Set to 1 to test term_read
#define TEST_TERM_WRITE 0 		//Set to 1 to test term_write
#define TEST_DIV0 	0           //Set to 1 to test divide by 0 exception
#define TEST_PAGEF 	0           //Set to 1 to test page fault exception
#define TEST_FS		0			//Set to 1 to test filesystem
#define TEST_RWRTC 	0           //Set to 1 to test rtc read/write

/* Check if MAGIC is valid and print the Multiboot information structure
   pointed by ADDR. */
void
entry (unsigned long magic, unsigned long addr)
{
    multiboot_info_t *mbi;

    /* Clear the screen. */
    clear();

    /* Am I booted by a Multiboot-compliant boot loader? */
    if (magic != MULTIBOOT_BOOTLOADER_MAGIC)
    {
        printf ("Invalid magic number: 0x%#x\n", (unsigned) magic);
        return;
    }

    /* Set MBI to the address of the Multiboot information structure. */
    mbi = (multiboot_info_t *) addr;

    /* Print out the flags. */
    printf ("flags = 0x%#x\n", (unsigned) mbi->flags);

    /* Are mem_* valid? */
    if (CHECK_FLAG (mbi->flags, 0))
        printf ("mem_lower = %uKB, mem_upper = %uKB\n",
                (unsigned) mbi->mem_lower, (unsigned) mbi->mem_upper);

    /* Is boot_device valid? */
    if (CHECK_FLAG (mbi->flags, 1))
        printf ("boot_device = 0x%#x\n", (unsigned) mbi->boot_device);

    /* Is the command line passed? */
    if (CHECK_FLAG (mbi->flags, 2))
        printf ("cmdline = %s\n", (char *) mbi->cmdline);

    if (CHECK_FLAG (mbi->flags, 3)) {
        int mod_count = 0;
        int i;
        module_t* mod = (module_t*)mbi->mods_addr;
        while(mod_count < mbi->mods_count) {
            printf("Module %d loaded at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_start);
            printf("Module %d ends at address: 0x%#x\n", mod_count, (unsigned int)mod->mod_end);
            printf("First few bytes of module:\n");
            for(i = 0; i<16; i++) {
                printf("0x%x ", *((char*)(mod->mod_start+i)));
            }
            printf("\n");
            mod_count++;
        }
    }
    /* Bits 4 and 5 are mutually exclusive! */
    if (CHECK_FLAG (mbi->flags, 4) && CHECK_FLAG (mbi->flags, 5))
    {
        printf ("Both bits 4 and 5 are set.\n");
        return;
    }

    /* Is the section header table of ELF valid? */
    if (CHECK_FLAG (mbi->flags, 5))
    {
        elf_section_header_table_t *elf_sec = &(mbi->elf_sec);

        printf ("elf_sec: num = %u, size = 0x%#x,"
                " addr = 0x%#x, shndx = 0x%#x\n",
                (unsigned) elf_sec->num, (unsigned) elf_sec->size,
                (unsigned) elf_sec->addr, (unsigned) elf_sec->shndx);
    }

    /* Are mmap_* valid? */
    if (CHECK_FLAG (mbi->flags, 6))
    {
        memory_map_t *mmap;

        printf ("mmap_addr = 0x%#x, mmap_length = 0x%x\n",
                (unsigned) mbi->mmap_addr, (unsigned) mbi->mmap_length);
        for (mmap = (memory_map_t *) mbi->mmap_addr;
                (unsigned long) mmap < mbi->mmap_addr + mbi->mmap_length;
                mmap = (memory_map_t *) ((unsigned long) mmap
                    + mmap->size + sizeof (mmap->size)))
            printf (" size = 0x%x,     base_addr = 0x%#x%#x\n"
                    "     type = 0x%x,  length    = 0x%#x%#x\n",
                    (unsigned) mmap->size,
                    (unsigned) mmap->base_addr_high,
                    (unsigned) mmap->base_addr_low,
                    (unsigned) mmap->type,
                    (unsigned) mmap->length_high,
                    (unsigned) mmap->length_low);
    }

    /* Construct an LDT entry in the GDT */
    {
        seg_desc_t the_ldt_desc;
        the_ldt_desc.granularity    = 0;
        the_ldt_desc.opsize         = 1;
        the_ldt_desc.reserved       = 0;
        the_ldt_desc.avail          = 0;
        the_ldt_desc.present        = 1;
        the_ldt_desc.dpl            = 0x0;
        the_ldt_desc.sys            = 0;
        the_ldt_desc.type           = 0x2;

        SET_LDT_PARAMS(the_ldt_desc, &ldt, ldt_size);
        ldt_desc_ptr = the_ldt_desc;
        lldt(KERNEL_LDT);
    }

    /* Construct a TSS entry in the GDT */
    {
        seg_desc_t the_tss_desc;
        the_tss_desc.granularity    = 0;
        the_tss_desc.opsize         = 0;
        the_tss_desc.reserved       = 0;
        the_tss_desc.avail          = 0;
        the_tss_desc.seg_lim_19_16  = TSS_SIZE & 0x000F0000;
        the_tss_desc.present        = 1;
        the_tss_desc.dpl            = 0x0;
        the_tss_desc.sys            = 0;
        the_tss_desc.type           = 0x9;
        the_tss_desc.seg_lim_15_00  = TSS_SIZE & 0x0000FFFF;

        SET_TSS_PARAMS(the_tss_desc, &tss, tss_size);

        tss_desc_ptr = the_tss_desc;

        tss.ldt_segment_selector = KERNEL_LDT;
        tss.ss0 = KERNEL_DS;
        tss.esp0 = 0x800000;
        ltr(KERNEL_TSS);
    }

    /* initialize paging */
    init_paging();
    
    /* Init the PIC */
    i8259_init();

    /* Initialize devices, memory, filesystem, enable device interrupts on the
     * PIC, any other initialization stuff... */
    init_idt();

    init_kbd();

    rtc_open();

    term_open();
	
		set_fs_loc((uint8_t*)(mbi->mods_addr), mbi->mods_count);

    /* Enable interrupts */
    /* Do not enable the following until after you have set up your
     * IDT correctly otherwise qemu will triple fault and simple close
     * without showing you any output */
    printf("enabling interrupts\n");

    sti();

#if TEST_HANDIN_CODE //set to 1 if wanna test the code Sam wrote during mp3.2 handin
    test_handin_code();
#endif

#if TEST_TERM_READ //set to 1 if wanna test term_read
	test_terminal_read();
#endif

#if TEST_TERM_WRITE //set to 1 if wanna test term_write
	test_terminal_write();
#endif

#if TEST_FS
    test_fs();
#endif

#if TEST_RWRTC //set to 1 to test rtc read/write
    test_rwrtc();
#endif

#if TEST_DIV0 //set to 1 to test divide by zero exception
    test_div0();
#endif

#if TEST_PAGEF //set to 1 to test page fault exception
    test_pagef();
#endif

    term_close();

    /* Execute the first program (`shell') ... */

    /* Spin (nicely, so we don't chew up cycles) */
    asm volatile(".1: hlt; jmp .1;");
}
