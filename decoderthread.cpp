#include "decoderthread.h"

#include "camerathread.h"

#include <QDebug>
#include <jpeglib.h>
#include <setjmp.h>
#include <turbojpeg.h>

DecoderThread::DecoderThread(CameraThread* liveviewThread, QObject *parent) : QThread(parent), m_cameraThread(liveviewThread), m_cameraFile(0)
{
    m_stop = false;
}


void DecoderThread::run()
{
    qInfo() << "Start liveview decoder thread";
    while (!m_stop) {
        m_mutex.lock();
        m_condition.wait(&m_mutex);
        m_mutex.unlock();

        if (!m_stop) {
            doDecodePreview();
        }
    }

    qInfo() << "Stop liveview decoder thread";
}

void DecoderThread::stop()
{
    m_mutex.lock();
    m_stop = true;
    m_condition.wakeOne();
    m_mutex.unlock();
}

void DecoderThread::doDecodePreview()
{
    unsigned long int size;
    const char *data;

    gp_file_get_data_and_size(m_cameraFile, &data, &size);

    QImage image = decodeImageTurbo(data, size);
    gp_file_free(m_cameraFile);
    m_cameraFile = 0;

    m_cameraThread->previewDecoded(image);
}

bool DecoderThread::decodePreview(CameraFile* cameraFile)
{
    if (m_mutex.tryLock()) {
        m_cameraFile = cameraFile;
        m_condition.wakeOne();
        m_mutex.unlock();
    }
}

struct my_error_mgr {
  struct jpeg_error_mgr pub;    /* "public" fields */

  jmp_buf setjmp_buffer;        /* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

/*
 * Here's the routine that will replace the standard error_exit method:
 */

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}





QImage DecoderThread::decodeImage(const char *data, unsigned long size)
{
    /* This struct contains the JPEG decompression parameters and pointers to
     * working space (which is allocated as needed by the JPEG library).
     */
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;

    /* More stuff */
    //JSAMPARRAY buffer;            /* Output row buffer */
    int row_stride;               /* physical row width in output buffer */

    /* Step 1: allocate and initialize JPEG decompression object */

    /* We set up the normal JPEG error routines, then override error_exit. */
    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
      /* If we get here, the JPEG code has signaled an error.
       * We need to clean up the JPEG object, close the input file, and return.
       */
      jpeg_destroy_decompress(&cinfo);
      return QImage(1, 1, QImage::Format_RGB888);
    }

    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */

    jpeg_mem_src(&cinfo, (unsigned char*) data, size);

    /* Step 3: read file parameters with jpeg_read_header() */

     (void) jpeg_read_header(&cinfo, TRUE);
    /* We can ignore the return value from jpeg_read_header since
     *   (a) suspension is not possible with the stdio data source, and
     *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
     * See libjpeg.txt for more info.
     */

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
     * jpeg_read_header(), so we do nothing here.
     */

    /* Step 5: Start decompressor */

    (void) jpeg_start_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* We may need to do some setup of our own at this point before reading
     * the data.  After jpeg_start_decompress() we have the correct scaled
     * output image dimensions available, as well as the output colormap
     * if we asked for color quantization.
     * In this example, we need to make an output work buffer of the right size.
     */
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    /*buffer = (*cinfo.mem->alloc_sarray)
                  ((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);*/

    QImage image = QImage(cinfo.output_width, cinfo.output_height, QImage::Format_RGB888);

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
     * loop counter, so that we don't have to keep track ourselves.
     */
    //qWarning() << "rowstride :" << row_stride << "output width" << cinfo.output_width << "output height :" << cinfo.output_height;
    int y = 0;
    //uchar* buffer = image.bits();
    while (cinfo.output_scanline < cinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
         * Here the array is only one element long, but you could ask for
         * more than one scanline at a time if that's more convenient.
         */

        uchar *row = image.scanLine(cinfo.output_scanline);
        (void) jpeg_read_scanlines(&cinfo, &row, 1);
/*
        for (int x = 0; x < cinfo.output_width; x += 3) {
            image.setPixel(x, y, (unsigned int) buffer[0][x]);
        }*/

        /* Assume put_scanline_someplace wants a pointer and sample count. */
        // TODO put_scanline_someplace(buffer[0], row_stride);
        y ++;
    }

    /* Step 7: Finish decompression */

    (void) jpeg_finish_decompress(&cinfo);
    /* We can ignore the return value since suspension is not possible
     * with the stdio data source.
     */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    return image;
}

QImage DecoderThread::decodeImageTurbo(const char *data, unsigned long size)
{
    int jpegSubsamp, width, height;

    tjhandle _jpegDecompressor = tjInitDecompress();
    tjDecompressHeader2(_jpegDecompressor, (uchar*) data, size, &width, &height, &jpegSubsamp);

    QImage image = QImage(width, height, QImage::Format_RGB888);

    tjDecompress2(_jpegDecompressor, (uchar*) data, size, image.bits(), width, 0/*pitch*/, height, TJPF_RGB, TJFLAG_FASTDCT);
    tjDestroy(_jpegDecompressor);

    return image;
}
