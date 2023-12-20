/*
 * Copyright (c) 2023 Paul D.Smith (paul@pauldsmith.org.uk).
 * License: Free to copy providing the author is acknowledged.
 *
 * Message strings stored just once to avoid duplication.
 */

#pragma once

const char *msg_creating  = "Creating tasks..."
const char *msg_start = "Task starting..."
const char *msg_waiting = "Task entering wait loop."
const char *msg_no_queue = "Unable to create message queue"
const char *msg_queue_not_ready = "Message queue is not ready"
const char *msg_send_failed  ="Message send failed"
const char *msg_msg_received = "Mesage received: %4.4hx";
const char *msg_flash_failed = "Get flash size failed";