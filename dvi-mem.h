#ifndef DVI_MEM
#define DVI_MEM

#define DVI_BASE_ADDRESS                0x80000000U
#define DVI_CONTROL_REGS_SIZE           16U

#define DVI_REGS_BUS_NAME               "BUS0"
#define DVI_VMEM_BUS_NAME               "VMEMBUS"

//The following values define the layout of the framebuffer memory used by the xilinx xps tft controller
#define DVI_VMEM_ADDRESS                (DVI_BASE_ADDRESS+DVI_CONTROL_REGS_SIZE)
#define DVI_VMEM_SIZE                   0x400000U //1024x576 pixels 32bpp 0xggbbrraa (notation in little endian)
#define DVI_VMEM_WIDTH                  1024
#define DVI_VMEM_HEIGHT                 512
#define DVI_VMEM_BYTES_PER_PIXEL        4
#define DVI_VMEM_BITS_PER_PIXEL         (DVI_VMEM_BYTES_PER_PIXEL*8)
#define DVI_VMEM_SCANLINE_BYTES         (DVI_VMEM_WIDTH*DVI_VMEM_BYTES_PER_PIXEL)
#define DVI_VMEM_RMASK                  0x00003f00
#define DVI_VMEM_GMASK                  0x003f0000
#define DVI_VMEM_BMASK                  0x3f000000

//despite the framebuffer itself is bigger, we only use/process the 640x480 portion of it (parameters like BPP and masks are same as above)
#define DVI_OUTPUT_WIDTH                640
#define DVI_OUTPUT_HEIGHT               480

#define DVI_TARGET_FPS 25

#define DVI_OUTPUT_SDL                  0
#define DVI_OUTPUT_STR_SDL              "sdl"
#define DVI_OUTPUT_DLO                  1
#define DVI_OUTPUT_STR_DLO              "dlo"
#define DVI_REDRAW_PTHREAD              0
#define DVI_REDRAW_PSE                  1
#define DVI_DISPLAY_DISABLED            0
#define DVI_DISPLAY_ENABLED             1
#define DVI_SCAN_TOP_BOTTOM             0
#define DVI_SCAN_BOTTOM_TOP             1

#define DVI_AR_OFFSET_BYTES             0
#define DVI_AR_OFFSET_INT               0
#define DVI_AR_ADDR                     (DVI_BASE_ADDRESS+DVI_AR_OFFSET_BYTES)
#define DVI_AR_ADDR_OFFSET              21 //TODO verify
#define DVI_AR_ADDR_MASK                0xffe00000

#define DVI_CR_OFFSET_INT               1
#define DVI_CR_OFFSET_BYTES             (4*DVI_CR_OFFSET_INT)
#define DVI_CR_ADDR                     (DVI_BASE_ADDRESS+DVI_CR_OFFSET_BYTES)
#define DVI_CR_EN_OFFSET                0
#define DVI_CR_EN_MASK                  (1 << DVI_CR_EN_OFFSET)
#define DVI_CR_DSC_OFFSET               1
#define DVI_CR_DSC_MASK                 (1 << DVI_CR_DSC_OFFSET)

#define DVI_IESR_OFFSET_INT             2
#define DVI_IESR_OFFSET_BYTES           (4*DVI_IESR_OFFSET_INT)
#define DVI_IESR_ADDR                   (DVI_BASE_ADDRESS+DVI_IESR_OFFSET_BYTES)
#define DVI_IESR_STATUS_OFFSET          0
#define DVI_IESR_STATUS_MASK            (1 << DVI_IESR_STATUS_OFFSET)
#define DVI_IESR_INTENABLE_OFFSET       3
#define DVI_IESR_INTENABLE_MASK         (1 << DVI_IESR_INTENABLE_OFFSET)

#endif //DVI_MEM
