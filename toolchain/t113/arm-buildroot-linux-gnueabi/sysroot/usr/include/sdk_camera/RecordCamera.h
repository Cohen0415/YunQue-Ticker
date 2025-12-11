#ifndef __RECORDCAMERA_H__
#define __RECORDCAMERA_H__

#include <signal.h>

#include <libcedarc/vencoder.h>
#include <libcedarx/CdxMuxer.h>
#include <libcedarx/CdxParser.h>

#include <sdk_camera/Rtc.h>
#include <sdk_camera/V4L2CameraDevice2.h>
#include <sdk_camera/CallbackNotifier.h>
#include <sdk_camera/PreviewWindow.h>
#include <sdk_camera/CameraHardware2.h>
#include <sdk_camera/MuxerWriter.h>
#include <sdk_camera/LegacyRecorder.h>

using namespace android;
/************Macro*********************/

#define ENC_USE_CAMERA_PHY_ADDR 0
#define DUMP_YUV 0
#define DUMP_VIDEOSTREAM 0
#define DUMP_COUNT 10

#define USE_CDX_LOWLEVEL_API 1
#define USE_CDX_LOWLEVEL_API_AUDIO 0
#define USE_RECORD_AUDIO_API 0


#define MSG_R 0400 /* read permission */
#define MSG_W 0200 /* write permission */
#define SVMSG_MODE (MSG_R | MSG_W | MSG_R >>3 | MSG_R >>6)
#define MQ_KEY  1234L
#define MSGMAX  (256)

#define CM_SAVE_FILE_MARK "save"
#define CM_LOCK_FILE_MARK "lock"
#define CM_MAX_RECORDFILE_NUM 400
#define CM_MAX_FILE_LEN 1024
#define CM_THUMB_DIR ".thumb/"
#define CAMERA_MSG_DVR_STORE_ERR  0x40000
#define CAMERA_MSG_DVR_NEW_FILE  0x80000

#define MAX_SUPPORT_CAM 2 //2  // mod lss

#define SECTION_CAMERA_PREFIX "camera"

#define SECTION_CAMERA0 "camera0"
#define SECTION_CAMERA1 "camera1"
#define SECTION_CAMERA360 "camera360"

#define CAMERA_NAME_LEN 16

#define CurRecordFileNameType_NORMAL 1
#define CurRecordFileNameType_LOCK 2
#define CurRecordFileNameType_SAVE 3

#define RecordStat_NORMAL 1
#define RecordStat_STOPCAR 2

#define LockOps_CreateNewNow 1
#define LockOps_CreateNewAfterClose 2

#define MISCWORK_CMD_RM 0XFF01
#define MISCWORK_CMD_SAVETHUMB 0XFF02
#define RECORD_CMD_TIMEOUT 0XFF03
#define RECORD_CMD_VIDEO 0XFF04
#define RECORD_CMD_AUDIO 0XFF05

#define ENC_RINGBUF_LEVEL 16
#define ENC_RINGBUF_DATALEN (300<<10)

// typedef void (*notify_callback)(int32_t msgType,
//                                 int32_t ext1,
//                                 int32_t ext2,
//                                 void* user);

// typedef void (*data_callback)(int32_t msgType,
//                               char *dataPtr,
//                               camera_frame_metadata_t *metadata,
//                               void* user);

// typedef void (*data_callback_timestamp)(nsecs_t timestamp,
//                                         int32_t msgType,
//                                         char *dataPtr,
//                                         void *user);
#ifdef ADAS_ENABLE
typedef void (*adas_data_callback)(int32_t msgType,
                                   char *dataPtr,
                                   void *metadata,
                                   void *user);
#endif

//message structure
typedef struct media_msg {
    long mesg_len;
    long mesg_type;
    nsecs_t timestamp;
    void *user;
    char mesg_data[MSGMAX];//muxter pkt
    int iskeyframe;//for video
    int resv;
    char *extradata;//audio data
} media_msg_t;

typedef struct {
    EXIFInfo                exifinfo;
    int                     quality;
    int                     jpeg_mode;
    VencJpegVideoSignal     vs;
    int                     jpeg_biteRate;
    int                     jpeg_frameRate;
    VencBitRateRange        bitRateRange;
    VencOverlayInfoS        sOverlayInfo;
    VencCopyROIConfig       pRoiConfig;
} jpeg_func_t;

typedef struct {
    char        intput_file[256];
    char        output_file[256];
    char        stream_file[256];
    char        mp4_file[256];
    unsigned int  encode_frame_num;
    unsigned int  encode_format;

    unsigned int src_size;
    unsigned int dst_size;

    unsigned int src_width;
    unsigned int src_height;
    unsigned int dst_width;
    unsigned int dst_height;

    unsigned int bit_rate;
    int frame_rate;
    int maxKeyFrame;

    unsigned char write_unmux;
    int muxer_type;
    long long pts;

    jpeg_func_t jpeg_func;
} encode_param_t;

enum RecordThreadState {
    /* Do not capture frame. */
    RECORD_STATE_NULL,
    /* Do not capture frame. */
    RECORD_STATE_PAUSED,
    /* Start capture frame. */
    RECORD_STATE_STARTED,
    /* exit thread*/
    RECORD_STATE_EXIT,
};

typedef struct file_status {
    int cur_file_idx;
    int cur_max_filenum;

    int timeout_second;
    char cur_filename[CM_MAX_RECORDFILE_NUM][CM_MAX_FILE_LEN];
    char cur_file_thumbname[CM_MAX_RECORDFILE_NUM][CM_MAX_FILE_LEN];//temp no use
    char cur_filedir[MAX_SUPPORT_CAM][CM_MAX_FILE_LEN];
    int cur_dir_file_num;
} file_status_t;

// typedef status_t (*usr_data_cb)(int32_t msgType,
//                                 char *dataPtr, int dalen,
//                                 void *user);

typedef struct ringbufQ {
    CdxMuxerPacketT         bufhead[ENC_RINGBUF_LEVEL];
    int                        write_id;
    int                        read_id;
    int                     read_id2;  //for muti rtsp client now support only one
    int                        buf_unused;
} ringbufQ_t;

typedef struct h264_encoder_parm {
    int bitrate;
    int in_width;
    int in_height;
    int out_width;
    int out_height;
    int framerate;
    VENC_PIXEL_FMT      eInputFormat;
    long long pts;
} h264_encoder_parm_t;

int init_ringbuf(ringbufQ_t *rbuf);
int destory_ringbuf(ringbufQ_t *rbuf);
CdxMuxerPacketT *push_ringbuf(ringbufQ_t *rbuf);
CdxMuxerPacketT *pop_ringbuf(ringbufQ_t *rbuf);

/************class******************/
class RecordCamera: public LegacyRecorder
{

public:
    RecordCamera(int cameraId);

    ~RecordCamera();

    int videoEncDeinit();

    int videoEncParmInit(int sw, int sh, int dw, int dh, unsigned int bitrate, int framerate,
                         int type, int pix = 0);

    int setDuration(int sec);

    int encode(V4L2BUF_t *pdata, char *outbuf, int *outsize, void *user);

    int start();

    int stop();

    int startRecord();

    int stopRecord();

    void setCallbacks(notify_callback notify_cb, void *user);

    void dataCallback(int32_t msgType, char *dataPtr, camera_frame_metadata_t *metadata, void *user);

    status_t dataCallbackTimestamp(nsecs_t timestamp, int32_t msgType, char *dataPtr, void *user);


    // static status_t audioRecMuxerCb(int32_t msgType,
    //                                 char *dataPtr, int dalen,
    //                                 void *user);
    int getRecordState();

    int SetDataCB(usr_data_cb cb, void *user);


private:

private:
    int getDuration();
    void setMaxKeyFrameInterval(int interval);
    void setForceKeyFrame(bool force);
    int setAudioMuxterType(AUDIO_ENCODER_TYPE aMuxerType);
    bool initFileListDir(char *dirname);
    int scan_picture(const char *path, int mCameraId);
    int generatepicFile();
    int generateThumbFile();
    int savePicture(void *data, int size, int fd);
    void dropQueue();
    bool recordThread();
    int awrecordwrite(media_msg_t   *pmsg);
    int closemux();
    int dvrsetvideoname(char *name);
private:
    enum RecordThreadState recordStat;
    bool mNeedExit;

    class RecordFileThread : public Thread
    {
        RecordCamera *cam;
        bool mRequestExit;
    public:

        RecordFileThread(RecordCamera *hw)
            : Thread(false), cam(hw), mRequestExit(false) { }

        void stopThread()
        {
            mRequestExit = true;
        }

        void startThread()
        {
            run("CameraRecordFileThread", PRIORITY_URGENT_DISPLAY);
        }

        virtual bool threadLoop()
        {
            if (mRequestExit) {
                ALOGW("record file thread exit");
                cam->dropQueue();
                return false;
            }

            return cam->recordThread();
        }
    };


public:
    pthread_mutex_t                 mRecordFileMutex;
    pthread_cond_t                    mRecordFileCond;
    OSAL_QUEUE                        mQueueBufferRecordFile;
    usr_data_cb                     mAudioRecMuxerCb;
    int mCameraId;
    int storage_state;
    VideoEncoder *pRecVideoEnc;


private:
    sp<RecordFileThread>            mRecordFileThread;
    usr_data_cb usrDatCb;
    void *mCbUserDat;


    static void __notify_cb(int32_t msg_type, int32_t ext1,
                            int32_t ext2, void *user)
    {
        ALOGV("recordcamera %s", __FUNCTION__);
        RecordCamera *__this = static_cast<RecordCamera *>(user);

        if (__this->mNotifyCb != NULL) {
            ALOGV("recordcamera %s line=%d cbdat=%p", __FUNCTION__, __LINE__, __this->mCbUser);
            __this->mNotifyCb(msg_type, ext1, ext2, __this->mCbUser);
        }
    }

    int sps_pps_flag; //use this flag to write sps_pps data to the new file

    int mVideoRecordQueueErr;
    int mAudioRecordQueueErr;
    AudioEncConfig mAudioCodecParm;
    int mNeedThumbpic;
    CdxFsCacheMemInfo fs_cache_mem;
    int fs_mode;

    //for rtsp process
    int start_rtsp();
    int stop_rtsp();
    int start_deliverrtsp(int disp_width, int disp_height);
    int stop_deliverrtsp();
    int destory_rtsp_encoder();
    int can_deliver_rtspdata;
    int rtsp_started;
    h264_encoder_parm_t rtsp_encparm;
    int encode_rtsp(V4L2BUF_t *pdata, char *outbuf, int *outsize, void *user, int freebuf);
private:
    Mutex mObjectLock;

    notify_callback mNotifyCb;
    void *mCbUser;

    file_status_t mFilemanger;
    long long FileNewStampBase;
    int mDuration;
    int mCaptureFmt;

    int iMaxPicNum;
    const char **ppSuffix;
    file_status_t *pFileManger;
    Mutex *pfileLock;

    //AutAudioEnc *audioEnc;

    int mAudioHdl;
    int mAudioflag;

    MuxerWriterT *mw;
    CdxMuxerT *mux;
    CdxDataSourceT mp4stream;
    CdxWriterT *writer;
    int AutAudioMuxInit(CdxMuxerMediaInfoT *pmedia_info);

    encode_param_t    encode_param;

    VencBaseConfig rec_enc_baseConfig;
    int enc_parm_destory(void);
    int setVideoEncParm();
    int mDvrWriteFrameNum;
    int mNeedNewFile;

    int camera_video_file_save(char *name, int filetype);
    int camera_change_file_name(int filetype);
    int setCurRecordFileLock();
    int setCurRecordFileSave();
    int setCurRecordFileNormal();
    //for 1 :normal for 2:stop car record
    int setCurRecordStat(int stat);
    int setCurRecordLockOps(int ops);
    int mCurRecordFileNameType;
    int mCurRecordStat;
    int mCurRecordLockOps;

    FILE *out_file ;

    int mTimeridVaild;
    int mCurRecordPos;
    timer_t  mTimerID;
    static void __timer_cb(sigval_t sig);

    int prepare();
    int genfilename(char *name);

    //below rtsp invaild
    VideoEncoder *pVideoEncRtsp;
    VencHeaderData rtsp_sps_pps;
    Mutex    mRtspLock;
    ringbufQ_t rtspQbuf;
    VencBaseConfig rtsp_enc_baseConfig;
    int istestrtsp;
    FILE *out_filertsp ;
    int create_rtsp_encoder(int bitrate, int in_width, int in_height, int out_width, int  out_height, int framerate);
};

#endif
