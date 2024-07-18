
//TODO: Replace Go callback with a C function wrapper that returns result to Go
//      Add equivalent navigation handling logic for macOS (Cocoa)

#include <stdlib.h>
#include <stdio.h>
#include "webview.h"

char *found_uri = "";

//Store URI in Go-accessible variable
void store_uri(char uri[]) {
    found_uri = uri;
}

#ifdef __linux__

#include <webkit2/webkit2.h>

static void loadChangedCb(WebKitWebView *web_view,
                                      WebKitLoadEvent load_event,
                                      gpointer        data){

        if (load_event == WEBKIT_LOAD_REDIRECTED) {
            gchar *redirected_uri = webkit_web_view_get_uri(web_view);

            store_uri(redirected_uri);
        }
    }

void linux_navigation_handler(webview_t web_view) {
    

    GtkWidget *handle = webview_get_native_handle(web_view, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER);
    g_signal_connect(WEBKIT_WEB_VIEW(handle), "load-changed", G_CALLBACK(loadChangedCb), NULL);
    return;
}

void onUriChange(webview_t web_view) {
    linux_navigation_handler(web_view);
}

// END __linux__
#elif defined __WIN32

#include <WebView2.h>
#include <string.h>

/* Forward declarations */

static ULONG HandlerRefCount = 0;
static ULONG HandlerAddRef(ICoreWebView2NavigationStartingEventHandler* This);
static ULONG HandlerRelease(ICoreWebView2NavigationStartingEventHandler* This);
static HRESULT HandlerQueryInterface(
    ICoreWebView2NavigationStartingEventHandler *This,
    REFIID riid,
        _COM_Outptr_  void **ppvObject
);
static HRESULT HandlerInvoke(
    ICoreWebView2NavigationStartingEventHandler* This,
    ICoreWebView2 *sender,
    ICoreWebView2NavigationStartingEventArgs* args
);

static ICoreWebView2NavigationStartingEventHandlerVtbl navigationHandlerVtbl = {
    HandlerQueryInterface,
    HandlerAddRef,
    HandlerRelease,
    HandlerInvoke
};
static ICoreWebView2NavigationStartingEventHandler navigationHandler = {
    &navigationHandlerVtbl
};


static ULONG HandlerAddRef(ICoreWebView2NavigationStartingEventHandler* This)
{
    return ++HandlerRefCount;
}

static ULONG HandlerRelease(ICoreWebView2NavigationStartingEventHandler* This)
{
    --HandlerRefCount;
    if (HandlerRefCount == 0)
    {
        /*if (&navigationHandler)
        {
            free(&navigationHandler.lpVtbl);
            free(&navigationHandler);
        }*/
    }
    return HandlerRefCount;
}
static HRESULT HandlerQueryInterface(
    ICoreWebView2NavigationStartingEventHandler *This,
    REFIID riid,
    _COM_Outptr_  void **ppvObject
)
{
    *ppvObject = This;
    HandlerAddRef(This);
    return S_OK;
}
static HRESULT HandlerInvoke(
    ICoreWebView2NavigationStartingEventHandler* This,
    ICoreWebView2 *sender,
    ICoreWebView2NavigationStartingEventArgs* args
)
{   
    char *temp = calloc(2048, sizeof(char));
    LPWSTR uri;
    args->lpVtbl->get_Uri(args, &uri);

    //Convert to regular char*
    wcstombs(temp, uri, 2048);

    if (!strncmp("qrc", temp, 3)) {
        found_uri=temp;
        //sender->lpVtbl->Release(sender);
        //m_controller->lpVtbl->Close(m_controller);
        //m_controller->lpVtbl->Release(m_controller);
    }
    
    // ! [NavigationKind]
    return S_OK;
}

void win32_navigation_handler(webview_t web_view) {

    ICoreWebView2Controller *m_controller = webview_get_native_handle(web_view, WEBVIEW_NATIVE_HANDLE_KIND_BROWSER_CONTROLLER);
    ICoreWebView2 *m_webview;
    m_controller->lpVtbl->get_CoreWebView2(m_controller, &m_webview);

    EventRegistrationToken token;

    m_webview->lpVtbl->AddRef(m_webview);
    m_webview->lpVtbl->add_NavigationStarting(m_webview, &navigationHandler,
        &token);
}

void onUriChange(webview_t web_view) {
    win32_navigation_handler(web_view);
}

// END __WIN32
#elif defined __APPLE__

//Store URI in Go-accessible variable
void store_uri(char uri[]) {
    found_uri = uri;
}

//TODO: Replace Go callback with a C function wrapper that returns result to Go
//      Add equivalent navigation handlers for macOS

void darwin_navigation_handler(webview_t web_view, void *cb) {

}

void onUriChange(webview_t web_view) {
    darwin_navigation_handler(web_view, store_uri);
}

// END __APPLE__
#else // Unsupported platform

void onUriChange(webview_t web_view) {
    printf("WebView navigation hook is not implemented on your platform.\n");
}

#endif