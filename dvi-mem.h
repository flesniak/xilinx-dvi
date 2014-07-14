#ifndef DVI_MEM
#define DVI_MEM

#define DVI_BASE_ADDRESS                0x80000000U
#define DVI_CONTROL_REGS_SIZE           16U
#define DVI_VMEM_ADDRESS                DVI_BASE_ADDRESS+DVI_CONTROL_REGS_SIZE
#define DVI_VMEM_SIZE                   0x400000U

#define DVI_VMEM_BYTES_PER_PIXEL        4
#define DVI_VMEM_SCANLINE               1024*DVI_VMEM_BYTES_PER_PIXEL
#define DVI_VMEM_RMASK                  0x00003f00
#define DVI_VMEM_GMASK                  0x003f0000
#define DVI_VMEM_BMASK                  0x3f000000

#define DVI_TARGET_FPS 25

#endif //DVI_MEM
