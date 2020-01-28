#ifndef __HTTPSERVER_H__
#define __HTTPSERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

void ATaskHttpServer( void *pvParameters );

void HttpServer_init(void);
void file_ok(int cfd,int len);



#ifdef __cplusplus
}
#endif

#endif

