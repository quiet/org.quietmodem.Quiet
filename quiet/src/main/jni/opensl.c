#include "quiet-jni.h"

SLresult quiet_opensl_system_create(quiet_opensl_system **sys_dest) {
    *sys_dest = calloc(1, sizeof(quiet_opensl_system));
    quiet_opensl_system *sys_deref = *sys_dest;

    SLresult res;

    SLObjectItf engine;
    SLEngineOption EngineOption[] = {
        {(SLuint32)SL_ENGINEOPTION_THREADSAFE, (SLuint32)SL_BOOLEAN_TRUE}};

    res = slCreateEngine(&engine, 1, EngineOption, 0, NULL, NULL);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    sys_deref->engine = engine;

    res = (*engine)->Realize(engine, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    SLEngineItf EngineItf;
    res = (*engine)->GetInterface(engine, SL_IID_ENGINE, (void *)&EngineItf);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    sys_deref->engine_itf = EngineItf;

    SLObjectItf OutputMix;

    res = (*EngineItf)->CreateOutputMix(EngineItf, &OutputMix, 0, NULL, NULL);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    sys_deref->output_mix = OutputMix;

    // realize this interface. block until it has realized (async = false)
    res = (*OutputMix)->Realize(OutputMix, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    return SL_RESULT_SUCCESS;
}

void quiet_opensl_system_destroy(quiet_opensl_system *sys) {
    if (sys->output_mix) {
        (*sys->output_mix)->Destroy(sys->output_mix);
        sys->output_mix = NULL;
    }

    if (sys->engine) {
        (*sys->engine)->Destroy(sys->engine);
        sys->engine = NULL;
        sys->engine_itf = NULL;
    }
    free(sys);
}

quiet_opensl_producer *opensl_producer_create(size_t num_buf,
                                              size_t num_frames) {
    quiet_opensl_producer *p = malloc(sizeof(quiet_opensl_producer));
    p->num_buf = num_buf;
    p->num_frames = num_frames;
    p->buf = malloc(p->num_buf * sizeof(opensl_sample_t *));
    p->buf_idx = 0;
    p->scratch = malloc(p->num_frames * sizeof(quiet_sample_t));
    for (size_t i = 0; i < p->num_buf; i++) {
        p->buf[i] = malloc(p->num_frames * num_playback_channels * sizeof(opensl_sample_t));
    }
    return p;
}

void opensl_producer_destroy(quiet_opensl_producer *p) {
    for (size_t i = 0; i < p->num_buf; i++) {
        free(p->buf[i]);
    }
    free(p->buf);
    free(p->scratch);
    free(p);
}

quiet_opensl_consumer *opensl_consumer_create(size_t num_buf,
                                              size_t num_frames) {
    quiet_opensl_consumer *c = malloc(sizeof(quiet_opensl_consumer));
    c->num_buf = num_buf;
    c->num_frames = num_frames;
    c->buf = malloc(c->num_buf * sizeof(opensl_sample_t *));
    c->buf_idx = 0;
    c->scratch = malloc(c->num_frames * sizeof(quiet_sample_t));
    for (size_t i = 0; i < c->num_buf; i++) {
        c->buf[i] = malloc(c->num_frames * num_record_channels * sizeof(opensl_sample_t));
    }
    return c;
}

void opensl_consumer_destroy(quiet_opensl_consumer *c) {
    for (size_t i = 0; i < c->num_buf; i++) {
        free(c->buf[i]);
    }
    free(c->buf);
    free(c->scratch);
    free(c);
}

void playback_callback(SLAndroidSimpleBufferQueueItf queueItf,
                       void *user_data) {
    quiet_opensl_player *player = (quiet_opensl_player *)user_data;
    quiet_opensl_producer *p = player->producer;
    ssize_t written = p->produce(p->produce_arg, p->scratch, p->num_frames);
    if (written == 0) {
        // EOF
        // a little dirty, but here we manage our queue and player
        SLAndroidSimpleBufferQueueState queue_state;
        (*queueItf)->GetState(queueItf, &queue_state);
        // we'll reach this point once for every buffer that was in flight
        // so e.g. with 3 buffers in the queue, we'll see this eof 3 times
        // we want to wait for the last one until we stop the player so that
        // everything has flushed
        if (queue_state.count == 0) {
            // this is the last callback, so let's shut down our player
            SLPlayItf play = player->play;
            (*play)->SetPlayState(play, SL_PLAYSTATE_STOPPED);
        }
        return;
    }
    if (written < 0) {
        // we'll assume this means no frames ready
        // in this case, we want to write a full block of silence
        // we need to call Enqueue() in order to get called back
        written = 0;
    }
    for (size_t i = written; i < p->num_frames; i++) {
        p->scratch[i] = 0;
    }
    size_t num_bytes = p->num_frames * num_playback_channels * sizeof(opensl_sample_t);
    memset(p->buf[p->buf_idx], 0, num_bytes);
    convert_monofloat2stereoopensl(p->scratch, p->buf[p->buf_idx],
                                   p->num_frames);

    SLresult res;
    // Enqueue() expects the number of bytes, not frames or samples
    res = (*queueItf)->Enqueue(queueItf, p->buf[p->buf_idx], num_bytes);
    if (res != SL_RESULT_SUCCESS) {
        // XXX
    }
    p->buf_idx = (p->buf_idx + 1) % p->num_buf;
}

SLresult quiet_opensl_create_player(quiet_opensl_system *sys,
                                    quiet_opensl_producer *p,
                                    quiet_opensl_player **player_dest) {
    *player_dest = calloc(1, sizeof(quiet_opensl_player));
    quiet_opensl_player *player_deref = *player_dest;

    SLresult res;

    SLDataLocator_AndroidSimpleBufferQueue bufferQueue;
    bufferQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    bufferQueue.numBuffers = p->num_buf;

#ifdef QUIET_JNI_USE_FLOAT
    SLAndroidDataFormat_PCM_EX pcm;
    pcm.formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
    pcm.bitsPerSample = 32;
    pcm.containerSize = 32;
    pcm.representation = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
    pcm.sampleRate = SL_SAMPLINGRATE_44_1;
#else
    SLDataFormat_PCM pcm;
    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize = 16;
    pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
#endif
    pcm.numChannels = 2;
    pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSource audioSource;
    audioSource.pFormat = (void *)&pcm;
    audioSource.pLocator = (void *)&bufferQueue;

    SLDataLocator_OutputMix locator_outputmix;
    locator_outputmix.locatorType = SL_DATALOCATOR_OUTPUTMIX;
    locator_outputmix.outputMix = sys->output_mix;

    SLDataSink audioSink;
    audioSink.pLocator = (void *)&locator_outputmix;
    audioSink.pFormat = NULL;

    SLObjectItf player;
    SLuint32 num_interfaces = num_record_channels;
    SLboolean required[num_interfaces];
    SLInterfaceID interfaces[num_interfaces];
    interfaces[0] = SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
    required[0] = SL_BOOLEAN_TRUE;
    res = (*sys->engine_itf)
              ->CreateAudioPlayer(sys->engine_itf, &player, &audioSource,
                                  &audioSink, 1, interfaces, required);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }
    player_deref->player = player;

    res = (*player)->Realize(player, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    SLPlayItf playItf;
    res = (*player)->GetInterface(player, SL_IID_PLAY, (void *)&playItf);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }
    player_deref->play = playItf;

    SLAndroidSimpleBufferQueueItf bufferQueueItf;
    res = (*player)->GetInterface(player, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                  (void *)&bufferQueueItf);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }
    player_deref->buffer_queue = bufferQueueItf;

    player_deref->producer = p;

    res = (*bufferQueueItf)
              ->RegisterCallback(bufferQueueItf, playback_callback, player_deref);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    for (size_t i = 0; i < p->num_buf; i++) {
        playback_callback(bufferQueueItf, player_deref);
    }

    res = (*playItf)->SetPlayState(playItf, SL_PLAYSTATE_PLAYING);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    return res;
}

SLresult quiet_opensl_stop_player(quiet_opensl_player *player) {
    SLPlayItf play = player->play;
    if (!play) {
        return SL_RESULT_SUCCESS;
    }
    SLresult res;

    res = (*play)->SetPlayState(play, SL_PLAYSTATE_STOPPED);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    SLuint32 play_state = SL_PLAYSTATE_PLAYING;
    while (play_state != SL_PLAYSTATE_STOPPED) {
        res = (*play)->GetPlayState(play, &play_state);
        if (res != SL_RESULT_SUCCESS) {
            return res;
        }
    }

    return SL_RESULT_SUCCESS;
}

void quiet_opensl_destroy_player(quiet_opensl_player *player) {
    if (player->player) {
        (*player->player)->Destroy(player->player);
        player->player = NULL;
        player->buffer_queue = NULL;
        player->play = NULL;
    }
    free(player);
}

void record_callback(SLAndroidSimpleBufferQueueItf queueItf, void *user_data) {
    quiet_opensl_recorder *recorder = (quiet_opensl_recorder *)user_data;
    quiet_opensl_consumer *c = (quiet_opensl_consumer *)recorder->consumer;
    convert_stereoopensl2monofloat(c->buf[c->buf_idx], c->scratch, c->num_frames, num_record_channels);
    c->consume(c->consume_arg, c->scratch, c->num_frames);

    SLresult res;
    size_t num_bytes = c->num_frames * num_record_channels * sizeof(opensl_sample_t);
    // Enqueue() expects the number of bytes, not frames or samples
    res = (*queueItf)->Enqueue(queueItf, c->buf[c->buf_idx], num_bytes);
    if (res != SL_RESULT_SUCCESS) {
        // XXX
    }
    c->buf_idx = (c->buf_idx + 1) % c->num_buf;
}

SLresult quiet_opensl_create_recorder(quiet_opensl_system *sys,
                                      quiet_opensl_consumer *c,
                                      quiet_opensl_recorder **recorder_dest) {
    *recorder_dest = calloc(1, sizeof(quiet_opensl_recorder));
    quiet_opensl_recorder *recorder_deref = *recorder_dest;

    SLresult res;

    SLDataLocator_IODevice loc_dev;
    loc_dev.locatorType = SL_DATALOCATOR_IODEVICE;
    loc_dev.deviceType = SL_IODEVICE_AUDIOINPUT;
    loc_dev.deviceID = SL_DEFAULTDEVICEID_AUDIOINPUT;
    loc_dev.device = NULL;

    SLDataSource audioSource;
    audioSource.pLocator = (void *)&loc_dev;
    audioSource.pFormat = NULL;

    SLDataLocator_AndroidSimpleBufferQueue bufferQueue;
    bufferQueue.locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
    bufferQueue.numBuffers = c->num_buf;

#ifdef QUIET_JNI_USE_FLOAT
    SLAndroidDataFormat_PCM_EX pcm;
    pcm.formatType = SL_ANDROID_DATAFORMAT_PCM_EX;
    pcm.bitsPerSample = 32;
    pcm.containerSize = 32;
    pcm.representation = SL_ANDROID_PCM_REPRESENTATION_FLOAT;
    pcm.sampleRate = SL_SAMPLINGRATE_44_1;
#else
    SLDataFormat_PCM pcm;
    pcm.formatType = SL_DATAFORMAT_PCM;
    pcm.bitsPerSample = SL_PCMSAMPLEFORMAT_FIXED_16;
    pcm.containerSize = 16;
    pcm.samplesPerSec = SL_SAMPLINGRATE_44_1;
#endif
    pcm.numChannels = num_record_channels;
    pcm.channelMask = SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT;
    pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

    SLDataSink audioSink;
    audioSink.pLocator = (void *)&bufferQueue;
    audioSink.pFormat = (void *)&pcm;

    SLObjectItf recorder;
    unsigned int num_interfaces = 1;
    SLInterfaceID interfaces[num_interfaces];
    SLboolean required[num_interfaces];
    interfaces[0] = SL_IID_ANDROIDSIMPLEBUFFERQUEUE;
    required[0] = SL_BOOLEAN_TRUE;
    res = (*sys->engine_itf)
              ->CreateAudioRecorder(sys->engine_itf, &recorder, &audioSource,
                                    &audioSink, 1, interfaces, required);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }
    recorder_deref->recorder = recorder;

    res = (*recorder)->Realize(recorder, SL_BOOLEAN_FALSE);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    SLRecordItf recordItf;
    res =
        (*recorder)->GetInterface(recorder, SL_IID_RECORD, (void *)&recordItf);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }
    recorder_deref->record = recordItf;

    SLAndroidSimpleBufferQueueItf bufferQueueItf;
    res = (*recorder)->GetInterface(recorder, SL_IID_ANDROIDSIMPLEBUFFERQUEUE,
                                    (void *)&bufferQueueItf);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }
    recorder_deref->buffer_queue = bufferQueueItf;

    recorder_deref->consumer = c;

    res =
        (*bufferQueueItf)->RegisterCallback(bufferQueueItf, record_callback, recorder_deref);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    size_t num_bytes = c->num_buf * num_record_channels * sizeof(opensl_sample_t);
    for (size_t i = 0; i < c->num_buf; i++) {
        // Enqueue() expects the number of bytes, not frames or samples
        res = (*bufferQueueItf)
                  ->Enqueue(bufferQueueItf, c->buf[i], num_bytes);
        if (res != SL_RESULT_SUCCESS) {
            return res;
        }
    }

    res = (*recordItf)->SetRecordState(recordItf, SL_RECORDSTATE_RECORDING);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    return res;
}

SLresult quiet_opensl_stop_recorder(quiet_opensl_recorder *recorder) {
    SLRecordItf record = recorder->record;
    if (!record) {
        return SL_RESULT_SUCCESS;
    }
    SLresult res;

    res = (*record)->SetRecordState(record, SL_RECORDSTATE_STOPPED);
    if (res != SL_RESULT_SUCCESS) {
        return res;
    }

    SLuint32 record_state = SL_RECORDSTATE_RECORDING;
    while (record_state != SL_RECORDSTATE_STOPPED) {
        res = (*record)->GetRecordState(record, &record_state);
        if (res != SL_RESULT_SUCCESS) {
            return res;
        }
    }

    return SL_RESULT_SUCCESS;
}

void quiet_opensl_destroy_recorder(quiet_opensl_recorder *recorder) {
    if (recorder->recorder) {
        (*recorder->recorder)->Destroy(recorder->recorder);
        recorder->recorder = NULL;
        recorder->buffer_queue = NULL;
        recorder->record = NULL;
    }
    free(recorder);
}
