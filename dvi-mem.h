#ifndef DVI_MEM
#define DVI_MEM

#define DVI_BASE_ADDRESS                0x80000000U
#define DVI_CONTROL_REGS_SIZE           16U
#define DVI_VMEM_ADDRESS                DVI_BASE_ADDRESS+DVI_CONTROL_REGS_SIZE
#define DVI_VMEM_SIZE                   0x400000U

#define DVI_REGS_BUS_NAME               "BUS0"
#define DVI_VMEM_BUS_NAME               "VMEMBUS"

#define DVI_VMEM_BYTES_PER_PIXEL        4
#define DVI_VMEM_SCANLINE               1024*DVI_VMEM_BYTES_PER_PIXEL
#define DVI_VMEM_RMASK                  0x00003f00
#define DVI_VMEM_GMASK                  0x003f0000
#define DVI_VMEM_BMASK                  0x3f000000

#define DVI_TARGET_FPS 25

#define DVI_OUTPUT_SDL                  0
#define DVI_OUTPUT_DLO                  1
#define DVI_REDRAW_PTHREAD              0
#define DVI_REDRAW_PSE                  1

#define DVI_AR_OFFSET                   0
#define DVI_AR_ADDR                     DVI_BASE_ADDRESS+DVI_AR_OFFSET
#define DVI_AR_ADDR_OFFSET              21 //TODO verify
#define DVI_AR_ADDR_MASK                0xffe00000

#define DVI_CR_OFFSET                   1
#define DVI_CR_ADDR                     DVI_BASE_ADDRESS+DVI_CR_OFFSET
#define DVI_CR_EN_OFFSET                0
#define DVI_CR_EN_MASK                  (1 << DVI_CR_EN_OFFSET)
#define DVI_CR_DSC_OFFSET               1
#define DVI_CR_DSC_MASK                 (1 << DVI_CR_DSC_OFFSET)

#define DVI_IESR_OFFSET                 2
#define DVI_IESR_ADDR                   DVI_BASE_ADDRESS+DVI_IESR_OFFSET
#define DVI_IESR_STATUS_OFFSET          0
#define DVI_IESR_STATUS_MASK            (1 << DVI_IESR_STATUS_OFFSET)
#define DVI_IESR_INTENABLE_OFFSET       3
#define DVI_IESR_INTENABLE_MASK         (1 << DVI_IESR_INTENABLE_OFFSET)

#endif //DVI_MEM
