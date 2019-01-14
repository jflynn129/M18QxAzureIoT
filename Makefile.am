AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = -I ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/inc/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/pal/linux/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/umqtt/inc/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/uamqp/inc/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/inc/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/inc/internal/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/deps/parson/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/serializer/inc/ \
           -I ./msft_azure_iot_sdk/azure-iot-sdk-c/platform/ \
           -I ./mbedtls/include/ \
           -O2 -Wall -static -fno-short-enums \
           -Wl,--unresolved-symbols=ignore-in-shared-libs 

AM_LDFLAGS = -static -ljson-c -lcurl -lpthread -lhw 

azIoTClient_CXXFLAGS = -std=c++14 -std=gnu++11 

libmsft_azure_iot_sdk_a_CFLAGS   = -Wno-unused-variable \
                                   -Wno-unused-function \
                                   -Wno-unknown-pragmas \
                                   -Wno-maybe-uninitialized \
                                   -Wno-enum-compare \
                                   -DENABLE_RAW_TRACE

libarmtls.a: 
	$(MAKE) -C mbedtls lib && \
        arm-oe-linux-gnueabi-ar cr libarmtls.a ./mbedtls/library/*.o

clean-generic:
	-$(MAKE) -C mbedtls clean

azIoTClient_SOURCES = azIoTClient.cpp foa.cpp mal.cpp jsmn.c i2c_interface.cpp \
                      hts221.cpp azClientFuncs.cpp azure_certs.c prettyjson.cpp\
                      lis2dw12.cpp button.cpp gps.cpp

noinst_LIBRARIES = libmsft_azure_iot_sdk.a libarmtls.a 


libmsft_azure_iot_sdk_a_SOURCES = ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransporthttp.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransporthttp.h \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_retry_control.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransportmqtt.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_mqtt_common.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_retry_control.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_amqp_connection.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransportamqp_methods.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/message_queue.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransportamqp.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_amqp_device.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_amqp_telemetry_messenger.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/uamqp_messaging.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_amqp_cbs_auth.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_amqp_twin_messenger.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_amqp_common.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_transport_ll_private.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport_amqp_messenger.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/blob.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_authorization.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_core.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_core_ll.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_diagnostic.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_ll.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_ll_uploadtoblob.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_client_retry_control.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_device_client.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_device_client_ll.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_message.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_module_client.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothub_module_client_ll.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/iothubtransport.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/message_queue.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/deps/parson/parson.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/iothub_client/src/version.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/base32.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/constmap.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/gb_stdio.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/httpheaders.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/sha224.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/usha.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/wsio.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/base64.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/crt_abstractions.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/gb_time.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/http_proxy_io.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/sha384-512.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/utf8_checker.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/xio.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/buffer.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/doublylinkedlist.c   \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/hmac.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/map.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/singlylinkedlist.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/uuid.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/xlogging.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/connection_string_parser.c   \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/gballoc.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/hmacsha256.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/optionhandler.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/strings.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/uws_client.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/consolelogger.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/gbnetwork.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/sastoken.c  \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/httpapiex.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/httpapiexsas.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/string_tokenizer.c   \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/uws_frame_encoder.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/constbuffer.c  \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/gb_rand.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/sha1.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/urlencode.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/vector.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/src/string_token.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/agenttime.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/httpapi_compact.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/httpapi_curl.c  \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/lock_pthreads.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/threadapi_pthreads.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/socketio_berkeley.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/umqtt/src/mqtt_client.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/umqtt/src/mqtt_codec.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/umqtt/src/mqtt_message.c \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/tickcounter_linux.c  \
                                  ./msft_azure_iot_sdk/azure-iot-sdk-c/c-utility/adapters/linux_time.c \
                                  ./msft_azure_iot_sdk/platform/tlsio_mbedtls.c \
                                  ./msft_azure_iot_sdk/platform/platform_linux.c  


bin_PROGRAMS = azIoTClient 

azIoTClient_LDADD = libmsft_azure_iot_sdk.a libarmtls.a


