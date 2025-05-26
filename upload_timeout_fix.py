Import("env")

env.Replace(
    UPLOADCMD="$PYTHONEXE $UPLOADER --chip esp32s3 --port $UPLOAD_PORT --baud $UPLOAD_SPEED write_flash 0x0 $SOURCE"
)


