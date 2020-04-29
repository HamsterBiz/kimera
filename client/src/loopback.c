#include "loopback.h"

bool open_loopback(LoopbackState* state, char* interface) {
	struct v4l2_format v;

    state->dev_fd = open(interface, O_RDWR);
	if (state->dev_fd == -1) {
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

	v.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	if (ioctl(state->dev_fd, VIDIOC_G_FMT, &v) == -1){
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

	v.fmt.pix.width = 1920;
	v.fmt.pix.height = 1080;
	v.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	v.fmt.pix.sizeimage = v.fmt.pix.width * v.fmt.pix.height * (3 / 2);
	v.fmt.pix.field = V4L2_FIELD_NONE;
	if (ioctl(state->dev_fd, VIDIOC_S_FMT, &v) == -1){
        printf("[LOOPBACK] Couldn't open interface.\n");
        return false;
	}

    state->buffer = (uint8_t*)malloc(v.fmt.pix.sizeimage);
    if (!state->buffer) {
        printf("[LOOPBACK] Couldn't allocate loopback buffer.\n");
        return false;
    }

	return true;
}

void close_loopback(LoopbackState* state) {
    if (state->buffer)
        free(state->buffer);
}

void loopback_push_frame(LoopbackState* state, AVFrame* frame) {
    size_t y_len = (frame->linesize[0] * frame->height);
    size_t u_len = (frame->linesize[1] * frame->height) / 2;
    size_t v_len = (frame->linesize[2] * frame->height) / 2;
    size_t len = y_len + u_len + v_len;

    memcpy(state->buffer, frame->data[0], y_len);
    memcpy(state->buffer + y_len, frame->data[1], u_len);
    memcpy(state->buffer + u_len + y_len, frame->data[2], v_len);

    write(state->dev_fd, state->buffer, len);
}