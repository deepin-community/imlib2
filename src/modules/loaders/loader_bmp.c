/*
 * Based off of Peter Alm's BMP loader from xmms, with additions from
 * imlib's old BMP loader
 */
/*
 * 21.3.2006 - Changes made by Petr Kobalicek
 * - Simplify and make secure RLE encoding
 * - Fix 16 and 32 bit depth (old code was incorrect and it's commented) 
 */
#include "loader_common.h"
#include <sys/stat.h>

#define DEBUG 0
#if DEBUG
#define D(fmt...) fprintf(stdout, "BMP loader: " fmt)
#else
#define D(fmt...)
#endif
#define Dx(fmt...)

/* The BITMAPFILEHEADER (size 14) */
typedef struct {
   DATA8               header[2];
   DATA8               size[4];
   DATA8               rsvd1[2];
   DATA8               rsvd2[2];
   DATA8               offs[4];
} bfh_t;

/* The BITMAPINFOHEADER */
typedef union {
   DATA32              header_size;
   struct {
      /* BITMAPCOREHEADER (size 12) */
      DATA32              header_size;
      DATA16              width;
      DATA16              height;
      DATA16              planes;
      DATA16              bpp;
   } bch;
   struct {
      /* BITMAPINFOHEADER (size 40) */
      DATA32              header_size;
      DATA32              width;
      DATA32              height;
      DATA16              planes;
      DATA16              bpp;
      DATA32              compression;
      DATA32              size;
      DATA32              res_hor;
      DATA32              res_ver;
      DATA32              colors;
      DATA32              colors_important;
      /* BITMAPV3INFOHEADER (size 56) */
      DATA32              mask_r;
      DATA32              mask_g;
      DATA32              mask_b;
      DATA32              mask_a;
   } bih;
   char                bytes[124];
} bih_t;

typedef struct {
   unsigned char       rgbBlue;
   unsigned char       rgbGreen;
   unsigned char       rgbRed;
   unsigned char       rgbReserved;
} RGBQUAD;

/* Compression methods */
#define BI_RGB                  0
#define BI_RLE8                 1
#define BI_RLE4                 2
#define BI_BITFIELDS            3
#define BI_JPEG                 4       /* Unsupported */
#define BI_PNG                  5       /* Unsupported */
#define BI_ALPHABITFIELDS       6
#define BI_CMYK                11       /* Unsupported */
#define BI_CMYKRLE8            12       /* Unsupported */
#define BI_CMYKRLE4            13       /* Unsupported */

enum {
   RLE_NEXT = 0,                /* Next line */
   RLE_END = 1,                 /* End of RLE encoding */
   RLE_MOVE = 2                 /* Move by X and Y (Offset is stored in two next bytes) */
};

static int
WriteleByte(FILE * file, unsigned char val)
{
   int                 rc;

   rc = fputc((int)val & 0xff, file);
   if (rc == EOF)
      return 0;

   return 1;
}

static int
WriteleShort(FILE * file, unsigned short val)
{
   int                 rc;

   rc = fputc((int)(val & 0xff), file);
   if (rc == EOF)
      return 0;
   rc = fputc((int)((val >> 8) & 0xff), file);
   if (rc == EOF)
      return 0;

   return 1;
}

static int
WriteleLong(FILE * file, unsigned long val)
{
   int                 rc;

   rc = fputc((int)(val & 0xff), file);
   if (rc == EOF)
      return 0;
   rc = fputc((int)((val >> 8) & 0xff), file);
   if (rc == EOF)
      return 0;
   rc = fputc((int)((val >> 16) & 0xff), file);
   if (rc == EOF)
      return 0;
   rc = fputc((int)((val >> 24) & 0xff), file);
   if (rc == EOF)
      return 0;

   return 1;
}

int
load2(ImlibImage * im, int load_data)
{
   int                 rc;
   unsigned int        offset;
   unsigned int        size, comp, imgsize;
   unsigned int        bitcount, ncols, skip;
   unsigned char       a, r, g, b;
   unsigned char       byte = 0, byte1, byte2;
   unsigned int        i, k;
   int                 w, h, x, y, j, l;
   DATA32             *ptr, pixel;
   unsigned char      *buffer_ptr, *buffer, *buffer_end, *buffer_end_safe;
   RGBQUAD             rgbQuads[256];
   DATA32              argbCmap[256];
   unsigned int        amask, rmask, gmask, bmask;
   int                 ashift1, rshift1, gshift1, bshift1;
   int                 ashift2, rshift2, gshift2, bshift2;
   bih_t               bih;

   rc = LOAD_FAIL;
   buffer = NULL;

   /* Load header */
   {
      struct stat         statbuf;
      bfh_t               bfh;

      if (fstat(fileno(im->fp), &statbuf) < 0)
         goto quit;

      size = statbuf.st_size;
      if ((long)size != statbuf.st_size)
         goto quit;

      if (fread(&bfh, sizeof(bfh), 1, im->fp) != 1)
         goto quit;

      if (bfh.header[0] != 'B' || bfh.header[1] != 'M')
         goto quit;

#define WORD_LE_32(p8) ((p8[3] << 24) | (p8[2] << 16) | (p8[1] << 8) | p8[0])
      offset = WORD_LE_32(bfh.offs);

      if (offset >= size)
         goto quit;

      memset(&bih, 0, sizeof(bih));
      if (fread(&bih, 4, 1, im->fp) != 1)
         goto quit;

      SWAP_LE_32_INPLACE(bih.header_size);

      D("fsize=%u, hsize=%u, header: fsize=%u offs=%u\n",
        size, bih.header_size, WORD_LE_32(bfh.size), offset);

      if (bih.header_size < 12 || bih.header_size > sizeof(bih))
         goto quit;

      if (fread(&bih.header_size + 1, bih.header_size - 4, 1, im->fp) != 1)
         goto quit;

      comp = BI_RGB;
      amask = rmask = gmask = bmask = 0;
      ashift1 = rshift1 = gshift1 = bshift1 = 0;
      ashift2 = rshift2 = gshift2 = bshift2 = 1;

      UNSET_FLAG(im->flags, F_HAS_ALPHA);

      if (bih.header_size == 12)
        {
           w = SWAP_LE_16(bih.bch.width);
           h = SWAP_LE_16(bih.bch.height);
//         planes = SWAP_LE_16(bih.bch.planes);
           bitcount = SWAP_LE_16(bih.bch.bpp);
        }
      else if (bih.header_size >= 16)
        {
           w = SWAP_LE_32(bih.bih.width);
           h = SWAP_LE_32(bih.bih.height);
//         planes = SWAP_LE_16(bih.bih.planes);
           bitcount = SWAP_LE_16(bih.bih.bpp);
           comp = SWAP_LE_32(bih.bih.compression);
//         imgsize = SWAP_LE_32(bih.bih.size);  /* We don't use this */

           if (bih.header_size >= 40 &&
               (comp == BI_BITFIELDS || comp == BI_ALPHABITFIELDS))
             {
                if (bih.header_size == 40)
                  {
                     ncols = (comp == BI_ALPHABITFIELDS) ? 4 : 3;
                     if (fread(&bih.bih.mask_r, 4, ncols, im->fp) != ncols)
                        goto quit;
                  }
                rmask = SWAP_LE_32(bih.bih.mask_r);
                gmask = SWAP_LE_32(bih.bih.mask_g);
                bmask = SWAP_LE_32(bih.bih.mask_b);
                amask = SWAP_LE_32(bih.bih.mask_a);
                if (amask)
                   SET_FLAG(im->flags, F_HAS_ALPHA);
             }
        }
      else
        {
           goto quit;
        }

      imgsize = size - offset;
      D("w=%3d h=%3d bitcount=%d comp=%d imgsize=%d\n",
        w, h, bitcount, comp, imgsize);

      /* "Bottom-up" images are loaded but not properly flipped */
      h = abs(h);

      if (!IMAGE_DIMENSIONS_OK(w, h))
         goto quit;

      switch (bitcount)
        {
        default:
           goto quit;

        case 1:
        case 4:
        case 8:
           ncols = (offset - bih.header_size - 14);
           if (bih.header_size == 12)
             {
                ncols /= 3;
                if (ncols > 256)
                   ncols = 256;
                for (i = 0; i < ncols; i++)
                   if (fread(&rgbQuads[i], 3, 1, im->fp) != 1)
                      goto quit;
             }
           else
             {
                ncols /= 4;
                if (ncols > 256)
                   ncols = 256;
                if (fread(rgbQuads, 4, ncols, im->fp) != ncols)
                   goto quit;
             }
           for (i = 0; i < ncols; i++)
              argbCmap[i] =
                 PIXEL_ARGB(0xff, rgbQuads[i].rgbRed, rgbQuads[i].rgbGreen,
                            rgbQuads[i].rgbBlue);
           D("ncols=%d\n", ncols);
           break;

        case 24:
           break;

        case 16:
        case 32:
           if (comp == BI_BITFIELDS || comp == BI_ALPHABITFIELDS)
             {
                unsigned int        bit, bithi;
                unsigned int        mask;

                D("mask   ARGB: %08x %08x %08x %08x\n",
                  amask, rmask, gmask, bmask);
                if (bitcount == 16)
                  {
                     amask &= 0xffffU;
                     rmask &= 0xffffU;
                     gmask &= 0xffffU;
                     bmask &= 0xffffU;
                  }
                if (rmask == 0 && gmask == 0 && bmask == 0)
                   goto quit;
                for (bit = 0; bit < bitcount; bit++)
                  {
                     /* Find LSB bit positions */
                     bithi = bitcount - bit - 1;
                     mask = 1 << bithi;
                     if (amask & mask)
                        ashift1 = bithi;
                     if (bmask & mask)
                        bshift1 = bithi;
                     if (gmask & mask)
                        gshift1 = bithi;
                     if (rmask & mask)
                        rshift1 = bithi;

                     /* Find MSB bit positions */
                     mask = 1 << bit;
                     if (amask & mask)
                        ashift2 = bit;
                     if (rmask & mask)
                        rshift2 = bit;
                     if (gmask & mask)
                        gshift2 = bit;
                     if (bmask & mask)
                        bshift2 = bit;
                  }

                /* Calculate shift2s as bits in mask */
                ashift2 -= ashift1 - 1;
                rshift2 -= rshift1 - 1;
                gshift2 -= gshift1 - 1;
                bshift2 -= bshift1 - 1;
             }
           else if (bitcount == 16)
             {
                rmask = 0x7C00;
                gmask = 0x03E0;
                bmask = 0x001F;
                rshift1 = 10;
                gshift1 = 5;
                bshift1 = 0;
                rshift2 = gshift2 = bshift2 = 5;
             }
           else if (bitcount == 32)
             {
                amask = 0xFF000000;
                rmask = 0x00FF0000;
                gmask = 0x0000FF00;
                bmask = 0x000000FF;
                ashift1 = 24;
                rshift1 = 16;
                gshift1 = 8;
                bshift1 = 0;
                ashift2 = rshift2 = gshift2 = bshift2 = 8;
             }

           /* Calculate shift2s as scale factor */
           ashift2 = ashift2 > 0 ? (1 << ashift2) - 1 : 1;
           rshift2 = rshift2 > 0 ? (1 << rshift2) - 1 : 1;
           gshift2 = gshift2 > 0 ? (1 << gshift2) - 1 : 1;
           bshift2 = bshift2 > 0 ? (1 << bshift2) - 1 : 1;

#define SCALE(c, x) ((((x & c##mask)>> (c##shift1 - 0)) * 255) / c##shift2)

           D("mask   ARGB: %08x %08x %08x %08x\n", amask, rmask, gmask, bmask);
           D("shift1 ARGB: %8d %8d %8d %8d\n",
             ashift1, rshift1, gshift1, bshift1);
           D("shift2 ARGB: %8d %8d %8d %8d\n",
             ashift2, rshift2, gshift2, bshift2);
           D("check  ARGB: %08x %08x %08x %08x\n",
             SCALE(a, amask), SCALE(r, rmask),
             SCALE(g, gmask), SCALE(b, bmask));
           break;
        }

      im->w = w;
      im->h = h;
   }

   if (!load_data)
     {
        rc = LOAD_SUCCESS;
        goto quit;
     }

   /* Load data */

   fseek(im->fp, offset, SEEK_SET);

   if (!__imlib_AllocateData(im))
      goto quit;

   buffer = malloc(imgsize);
   if (!buffer)
      goto quit;

   if (fread(buffer, imgsize, 1, im->fp) != 1)
      goto quit;

   buffer_ptr = buffer;
   buffer_end = buffer + imgsize;

   ptr = im->data + ((h - 1) * w);

   switch (bitcount)
     {
     default:                  /* It should not be possible to go here */
        goto quit;

     case 1:
        switch (comp)
          {
          default:
             goto quit;

          case BI_RGB:
             skip = ((((w + 31) / 32) * 32) - w) / 8;
             for (y = 0; y < h; y++)
               {
                  for (x = 0; x < w && buffer_ptr < buffer_end; x++)
                    {
                       if ((x & 7) == 0)
                          byte = *buffer_ptr++;
                       k = (byte >> 7) & 1;
                       *ptr++ = argbCmap[k];
                       byte <<= 1;
                    }
                  buffer_ptr += skip;
                  ptr -= w * 2;

                  if (im->lc && __imlib_LoadProgressRows(im, h - y - 1, -1))
                    {
                       rc = LOAD_BREAK;
                       goto quit;
                    }
               }
             break;
          }
        break;

     case 4:
        switch (comp)
          {
          default:
             goto quit;

          case BI_RLE4:
             buffer_end_safe = buffer_end - 1;

             x = 0;
             y = 0;

             for (; buffer_ptr < buffer_end_safe;)
               {
                  byte1 = buffer_ptr[0];
                  byte2 = buffer_ptr[1];
                  buffer_ptr += 2;
                  Dx("%3d %3d: %02x %02x (%d %d)\n",
                     x, y, byte1, byte2, byte2 >> 4, byte2 & 0xf);
                  if (byte1)
                    {
                       DATA32              t1, t2;

                       l = byte1;
                       /* Check for invalid length */
                       if (l + x > w)
                          goto bail_bc4;

                       t1 = argbCmap[byte2 >> 4];
                       t2 = argbCmap[byte2 & 0xF];
                       for (j = 0; j < l; j++)
                         {
                            *ptr++ = t1;
                            if (++j < l)
                               *ptr++ = t2;
                         }
                       x += l;
                    }
                  else
                    {
                       switch (byte2)
                         {
                         case RLE_NEXT:
                            x = 0;
                            if (++y >= h)
                               goto bail_bc4;
                            ptr = im->data + (h - y - 1) * w;
                            break;
                         case RLE_END:
                            x = 0;
                            y = h;
                            buffer_ptr = buffer_end_safe;
                            break;
                         case RLE_MOVE:
                            /* Need to read two bytes */
                            if (buffer_ptr >= buffer_end_safe)
                               goto bail_bc4;
                            x += buffer_ptr[0];
                            y += buffer_ptr[1];
                            buffer_ptr += 2;
                            /* Check for correct coordinates */
                            if (x >= w)
                               goto bail_bc4;
                            if (y >= h)
                               goto bail_bc4;
                            ptr = im->data + (h - y - 1) * w + x;
                            break;
                         default:
                            l = byte2;
                            /* Check for invalid length and valid buffer size */
                            if (l + x > w)
                               goto bail_bc4;
                            if (buffer_ptr + (l >> 1) + (l & 1) > buffer_end)
                               goto bail_bc4;

                            for (j = 0; j < l; j++)
                              {
                                 byte = *buffer_ptr++;
                                 Dx("%3d %3d:   %d/%d: %2d %2d\n",
                                    x, y, j, l, byte >> 4, byte & 0xf);
                                 *ptr++ = argbCmap[byte >> 4];
                                 if (++j < l)
                                    *ptr++ = argbCmap[byte & 0xF];
                              }
                            x += l;

                            /* Pad to even number of palette bytes */
                            buffer_ptr += ((l + 1) / 2) & 1;
                            break;
                         }
                    }
                  goto progress_bc4;

                bail_bc4:
                  buffer_ptr = buffer_end_safe;

                progress_bc4:
                  if (im->lc && (x == w) &&
                      __imlib_LoadProgressRows(im, h - y - 1, -1))
                    {
                       rc = LOAD_BREAK;
                       goto quit;
                    }
               }
             break;

          case BI_RGB:
             skip = ((((w + 7) / 8) * 8) - w) / 2;
             for (y = 0; y < h; y++)
               {
                  for (x = 0; x < w && buffer_ptr < buffer_end; x++)
                    {
                       if ((x & 1) == 0)
                          byte = *buffer_ptr++;
                       k = (byte & 0xF0) >> 4;
                       *ptr++ = argbCmap[k];
                       byte <<= 4;
                    }
                  buffer_ptr += skip;
                  ptr -= w * 2;

                  if (im->lc && __imlib_LoadProgressRows(im, h - y - 1, -1))
                    {
                       rc = LOAD_BREAK;
                       goto quit;
                    }
               }
             break;
          }
        break;

     case 8:
        switch (comp)
          {
          default:
             goto quit;

          case BI_RLE8:
             buffer_end_safe = buffer_end - 1;

             x = 0;
             y = 0;
             for (; buffer_ptr < buffer_end_safe;)
               {
                  byte1 = buffer_ptr[0];
                  byte2 = buffer_ptr[1];
                  buffer_ptr += 2;
                  Dx("%3d %3d: %02x %02x\n", x, y, byte1, byte2);
                  if (byte1)
                    {
                       pixel = argbCmap[byte2];
                       l = byte1;
                       if (x + l > w)
                          goto bail_bc8;
                       for (j = l; j; j--)
                          *ptr++ = pixel;
                       x += l;
                    }
                  else
                    {
                       switch (byte2)
                         {
                         case RLE_NEXT:
                            x = 0;
                            if (++y >= h)
                               goto bail_bc8;
                            ptr = im->data + ((h - y - 1) * w) + x;
                            break;
                         case RLE_END:
                            x = 0;
                            y = h;
                            buffer_ptr = buffer_end_safe;
                            break;
                         case RLE_MOVE:
                            /* Need to read two bytes */
                            if (buffer_ptr >= buffer_end_safe)
                               goto bail_bc8;
                            x += buffer_ptr[0];
                            y += buffer_ptr[1];
                            buffer_ptr += 2;
                            /* Check for correct coordinates */
                            if (x >= w)
                               goto bail_bc8;
                            if (y >= h)
                               goto bail_bc8;
                            ptr = im->data + ((h - y - 1) * w) + x;
                            break;
                         default:
                            l = byte2;
                            if (x + l > w)
                               goto bail_bc8;
                            if (buffer_ptr + l > buffer_end)
                               goto bail_bc8;
                            for (j = 0; j < l; j++)
                              {
                                 byte = *buffer_ptr++;
                                 Dx("%3d %3d:   %d/%d: %2d\n",
                                    x, y, j, l, byte);
                                 *ptr++ = argbCmap[byte];
                              }
                            x += l;
                            if (l & 1)
                               buffer_ptr++;
                            break;
                         }
                    }
                  goto progress_bc8;

                bail_bc8:
                  buffer_ptr = buffer_end_safe;

                progress_bc8:
                  if (im->lc && (x == w) &&
                      __imlib_LoadProgressRows(im, h - y - 1, -1))
                    {
                       rc = LOAD_BREAK;
                       goto quit;
                    }
               }
             break;

          case BI_RGB:
             skip = (((w + 3) / 4) * 4) - w;
             for (y = 0; y < h; y++)
               {
                  for (x = 0; x < w && buffer_ptr < buffer_end; x++)
                    {
                       byte = *buffer_ptr++;
                       *ptr++ = argbCmap[byte];
                    }
                  ptr -= w * 2;
                  buffer_ptr += skip;

                  if (im->lc && __imlib_LoadProgressRows(im, h - y - 1, -1))
                    {
                       rc = LOAD_BREAK;
                       goto quit;
                    }
               }
             break;
          }
        break;

     case 16:
        buffer_end_safe = buffer_end - 1;

        skip = (((w * 16 + 31) / 32) * 4) - (w * 2);
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w && buffer_ptr < buffer_end_safe; x++)
               {
                  pixel = *(unsigned short *)buffer_ptr;

                  if (im->flags & F_HAS_ALPHA)
                     a = SCALE(a, pixel);
                  else
                     a = 0xff;
                  r = SCALE(r, pixel);
                  g = SCALE(g, pixel);
                  b = SCALE(b, pixel);
                  *ptr++ = PIXEL_ARGB(a, r, g, b);
                  buffer_ptr += 2;
               }
             ptr -= w * 2;
             buffer_ptr += skip;

             if (im->lc && __imlib_LoadProgressRows(im, h - y - 1, -1))
               {
                  rc = LOAD_BREAK;
                  goto quit;
               }
          }
        break;

     case 24:
        buffer_end_safe = buffer_end - 2;

        skip = (4 - ((w * 3) % 4)) & 3;
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w && buffer_ptr < buffer_end_safe; x++)
               {
                  b = *buffer_ptr++;
                  g = *buffer_ptr++;
                  r = *buffer_ptr++;
                  *ptr++ = PIXEL_ARGB(0xff, r, g, b);
               }
             ptr -= w * 2;
             buffer_ptr += skip;

             if (im->lc && __imlib_LoadProgressRows(im, h - y - 1, -1))
               {
                  rc = LOAD_BREAK;
                  goto quit;
               }
          }
        break;

     case 32:
        buffer_end_safe = buffer_end - 3;

        skip = (((w * 32 + 31) / 32) * 4) - (w * 4);
        for (y = 0; y < h; y++)
          {
             for (x = 0; x < w && buffer_ptr < buffer_end_safe; x++)
               {
                  pixel = *(unsigned int *)buffer_ptr;

                  if (im->flags & F_HAS_ALPHA)
                     a = SCALE(a, pixel);
                  else
                     a = 0xff;
                  r = SCALE(r, pixel);
                  g = SCALE(g, pixel);
                  b = SCALE(b, pixel);
                  *ptr++ = PIXEL_ARGB(a, r, g, b);
                  buffer_ptr += 4;
               }
             ptr -= w * 2;
             buffer_ptr += skip;

             if (im->lc && __imlib_LoadProgressRows(im, h - y - 1, -1))
               {
                  rc = LOAD_BREAK;
                  goto quit;
               }
          }
        break;
     }

   rc = LOAD_SUCCESS;

 quit:
   if (rc <= 0)
      __imlib_FreeData(im);
   free(buffer);

   return rc;
}

char
save(ImlibImage * im, ImlibProgressFunction progress, char progress_granularity)
{
   int                 rc;
   FILE               *f;
   int                 i, j, pad;
   DATA32              pixel;

   f = fopen(im->real_file, "wb");
   if (!f)
      return LOAD_FAIL;

   rc = LOAD_SUCCESS;

   /* calculate number of bytes to pad on end of each row */
   pad = (4 - ((im->w * 3) % 4)) & 0x03;

   /* write BMP file header */
   WriteleShort(f, 0x4d42);     /* prefix */
   WriteleLong(f, 54 + ((3 * im->w) + pad) * im->h);    /* filesize (padding should be considered) */
   WriteleShort(f, 0x0000);     /* reserved #1 */
   WriteleShort(f, 0x0000);     /* reserved #2 */
   WriteleLong(f, 54);          /* offset to image data */

   /* write BMP bitmap header */
   WriteleLong(f, 40);          /* 40-byte header */
   WriteleLong(f, im->w);
   WriteleLong(f, im->h);
   WriteleShort(f, 1);          /* one plane      */
   WriteleShort(f, 24);         /* bits per pixel */
   WriteleLong(f, 0);           /* no compression */
   WriteleLong(f, ((3 * im->w) + pad) * im->h); /* padding should be counted */
   for (i = 0; i < 4; i++)
      WriteleLong(f, 0x0000);   /* pad to end of header */

   /* write actual BMP data */
   for (i = 0; i < im->h; i++)
     {
        for (j = 0; j < im->w; j++)
          {
             pixel = im->data[im->w * (im->h - i - 1) + j];
             WriteleByte(f, PIXEL_B(pixel));
             WriteleByte(f, PIXEL_G(pixel));
             WriteleByte(f, PIXEL_R(pixel));
          }
        for (j = 0; j < pad; j++)
           WriteleByte(f, 0);
     }

   fclose(f);

   return rc;
}

void
formats(ImlibLoader * l)
{
   static const char  *const list_formats[] = { "bmp" };
   __imlib_LoaderSetFormats(l, list_formats,
                            sizeof(list_formats) / sizeof(char *));
}
