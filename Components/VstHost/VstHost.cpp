#include <VstHost.h>

#include <pluginterfaces/vst2.x/aeffectx.h>

using namespace DSPatch;
using namespace DSPatchables;

typedef AEffect* ( *PluginEntryProc )( audioMasterCallback audioMaster );
static VstIntPtr VSTCALLBACK
    HostCallback( AEffect* effect, unsigned short opcode, unsigned short index, VstIntPtr value, void* ptr, float opt );

#if _WIN32

#include <windows.h>

struct VST_DLGTEMPLATE : DLGTEMPLATE
{
    WORD ext[3];
    VST_DLGTEMPLATE()
    {
        memset( this, 0, sizeof( *this ) );
    };
};

static INT_PTR CALLBACK EditorProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam );
static AEffect* vstEffect = NULL;
static HWND vstHwnd;

INT_PTR CALLBACK EditorProc( HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
    vstHwnd = hwnd;

    switch ( msg )
    {
        case WM_INITDIALOG:
        {
            SetWindowText( hwnd, "VST Editor" );
            SetTimer( hwnd, 1, 20, 0 );

            if ( vstEffect )
            {
                printf( "HOST> Open editor...\n" );
                vstEffect->dispatcher( vstEffect, effEditOpen, 0, 0, hwnd, 0 );

                printf( "HOST> Get editor rect..\n" );
                ERect* eRect = 0;
                vstEffect->dispatcher( vstEffect, effEditGetRect, 0, 0, &eRect, 0 );
                if ( eRect )
                {
                    int width = eRect->right - eRect->left;
                    int height = eRect->bottom - eRect->top;

                    if ( width < 100 )
                    {
                        width = 100;
                    }
                    if ( height < 100 )
                    {
                        height = 100;
                    }

                    RECT wRect;
                    SetRect( &wRect, 0, 0, width, height );
                    AdjustWindowRectEx( &wRect, GetWindowLong( hwnd, GWL_STYLE ), FALSE,
                                        GetWindowLong( hwnd, GWL_EXSTYLE ) );
                    width = wRect.right - wRect.left;
                    height = wRect.bottom - wRect.top;

                    SetWindowPos( hwnd, HWND_TOP, 0, 0, width, height, SWP_NOMOVE );
                }
            }
        }
        break;

        case WM_TIMER:
            if ( vstEffect )
            {
                vstEffect->dispatcher( vstEffect, effEditIdle, 0, 0, 0, 0 );
            }
            break;

        case WM_CLOSE:
        {
            KillTimer( hwnd, 1 );

            printf( "HOST> Close editor..\n" );
            if ( vstEffect )
            {
                vstEffect->dispatcher( vstEffect, effEditClose, 0, 0, 0, 0 );
            }

            EndDialog( hwnd, IDOK );
        }
        break;
    }

    return 0;
}

#elif TARGET_API_MAC_CARBON

#include <Carbon/Carbon.h>
static pascal OSStatus windowHandler( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData );
static pascal void idleTimerProc( EventLoopTimerRef inTimer, void* inUserData );

pascal void idleTimerProc( EventLoopTimerRef inTimer, void* inUserData )
{
    AEffect* effect = (AEffect*)inUserData;
    effect->dispatcher( effect, effEditIdle, 0, 0, 0, 0 );
}

pascal OSStatus windowHandler( EventHandlerCallRef inHandlerCallRef, EventRef inEvent, void* inUserData )
{
    OSStatus result = eventNotHandledErr;
    WindowRef window = (WindowRef)inUserData;
    UInt32 eventClass = GetEventClass( inEvent );
    UInt32 eventKind = GetEventKind( inEvent );

    switch ( eventClass )
    {
        case kEventClassWindow:
        {
            switch ( eventKind )
            {
                case kEventWindowClose:
                {
                    QuitAppModalLoopForWindow( window );
                    break;
                }
            }
            break;
        }
    }

    return result;
}

#endif

class VstLoader
{
public:
    VstLoader()
        : _module( 0 )
    {
    }

    ~VstLoader()
    {
        if ( _module )
        {
#if _WIN32

            FreeLibrary( (HMODULE)_module );

#elif TARGET_API_MAC_CARBON

            CFBundleUnloadExecutable( (CFBundleRef)_module );
            CFRelease( (CFBundleRef)_module );

#endif
        }
    }

    bool LoadVst( const char* fileName )
    {
#if _WIN32

        _module = LoadLibrary( fileName );

#elif TARGET_API_MAC_CARBON

        CFStringRef fileNameString = CFStringCreateWithCString( NULL, fileName, kCFStringEncodingUTF8 );
        if ( fileNameString == 0 )
        {
            return false;
        }

        CFURLRef url = CFURLCreateWithFileSystemPath( NULL, fileNameString, kCFURLPOSIXPathStyle, false );
        CFRelease( fileNameString );
        if ( url == 0 )
        {
            return false;
        }

        _module = CFBundleCreate( NULL, url );
        CFRelease( url );
        if ( _module && CFBundleLoadExecutable( (CFBundleRef)_module ) == false )
        {
            return false;
        }

#endif

        return _module != 0;
    }

    PluginEntryProc GetMainEntry()
    {
        PluginEntryProc mainProc = 0;

#if _WIN32

        mainProc = (PluginEntryProc)GetProcAddress( (HMODULE)_module, "VSTPluginMain" );
        if ( !mainProc )
        {
            mainProc = (PluginEntryProc)GetProcAddress( (HMODULE)_module, "main" );
        }

#elif TARGET_API_MAC_CARBON

        mainProc = (PluginEntryProc)CFBundleGetFunctionPointerForName( (CFBundleRef)_module, CFSTR( "VSTPluginMain" ) );
        if ( !mainProc )
        {
            mainProc =
                (PluginEntryProc)CFBundleGetFunctionPointerForName( (CFBundleRef)_module, CFSTR( "main_macho" ) );
        }

#endif

        return mainProc;
    }

private:
    void* _module;
};

VstIntPtr VSTCALLBACK
    HostCallback( AEffect* effect, VstInt32 opcode, VstInt32 index, VstIntPtr value, void* ptr, float opt )
{
    VstIntPtr result = 0;

    // Filter idle calls...
    bool filtered = false;

    if ( opcode == audioMasterIdle )
    {
        static bool wasIdle = false;
        if ( wasIdle )
            filtered = true;
        else
        {
            printf( "(Future idle calls will not be displayed!)\n" );
            wasIdle = true;
        }
    }

    if ( !filtered )
    {
        printf( "PLUG> HostCallback (opcode %d)\n index = %d, value = %p, ptr = %p, opt = %f\n", opcode, index,
                FromVstPtr<void>( value ), ptr, opt );
    }

    switch ( opcode )
    {
        case audioMasterVersion:
            result = kVstVersion;
            break;
    }

    return result;
}

VstHost::VstHost()
    : _vstInputs( NULL )
    , _vstOutputs( NULL )
    , _bufferSize( 128 )
    , _vstLoader( new VstLoader() )
    , _vstEffect( NULL )
{
    vstHwnd = NULL;
    LoadVst( "surrounddelay.dll" );
    ShowVst();
}

VstHost::~VstHost()
{
    CloseVst();
    delete _vstLoader;
}

bool VstHost::LoadVst( const char* vstPath )
{
    // close current VST
    CloseVst();

    // load new VST from vstPath
    if ( !_vstLoader->LoadVst( vstPath ) )
    {
        return false;
    }

    // get VST entry point
    PluginEntryProc mainEntry = _vstLoader->GetMainEntry();
    if ( !mainEntry )
    {
        return false;
    }

    // get VST effect handle
    _vstEffect = mainEntry( HostCallback );
    if ( !_vstEffect )
    {
        return false;
    }

    // configure component IO
    if ( _vstEffect->numInputs > 0 )
    {
        _vstInputs = new float*[_vstEffect->numInputs];
        for ( long i = 0; i < _vstEffect->numInputs; i++ )
        {
            _vstInputs[i] = new float[_bufferSize];
            memset( _vstInputs[i], 0, _bufferSize * sizeof( float ) );
        }
    }

    if ( _vstEffect->numOutputs > 0 )
    {
        _vstOutputs = new float*[_vstEffect->numOutputs];
        for ( long i = 0; i < _vstEffect->numOutputs; i++ )
        {
            _vstOutputs[i] = new float[_bufferSize];
            memset( _vstOutputs[i], 0, _bufferSize * sizeof( float ) );
        }
    }

    _dspInputs.resize( _vstEffect->numInputs );
    _dspOutputs.resize( _vstEffect->numOutputs );

    for ( long i = 0; i < _vstEffect->numInputs; i++ )
    {
        AddInput_();
        _dspInputs[i].resize( _bufferSize );
    }
    for ( long i = 0; i < _vstEffect->numOutputs; i++ )
    {
        AddOutput_();
        _dspOutputs[i].resize( _bufferSize );
    }

    // open and configure VST
    _vstEffect->dispatcher( _vstEffect, effOpen, 0, 0, 0, 0 );
    _vstEffect->dispatcher( _vstEffect, effSetSampleRate, 0, 0, 0, 44100 );
    _vstEffect->dispatcher( _vstEffect, effSetBlockSize, 0, _bufferSize, 0, 0 );

    _vstEffect->dispatcher( _vstEffect, effMainsChanged, 0, 1, 0, 0 );

    return true;
}

void VstHost::CloseVst()
{
    // continue only if a VST is currently open
    if ( _vstEffect )
    {
        HideVst();

        // inform VST that processing has seized
        _vstEffect->dispatcher( _vstEffect, effMainsChanged, 0, 0, 0, 0 );

        // clear all component IO
        if ( _vstEffect->numInputs > 0 )
        {
            for ( long i = 0; i < _vstEffect->numInputs; i++ )
            {
                delete[] _vstInputs[i];
            }
            delete[] _vstInputs;
            _vstInputs = NULL;
        }

        if ( _vstEffect->numOutputs > 0 )
        {
            for ( long i = 0; i < _vstEffect->numOutputs; i++ )
            {
                delete[] _vstOutputs[i];
            }
            delete[] _vstOutputs;
            _vstOutputs = NULL;
        }

        // close VST
        _vstEffect->dispatcher( _vstEffect, effClose, 0, 0, 0, 0 );
        _vstEffect = NULL;

        ClearInputs_();
        ClearOutputs_();
    }
}

bool VstHost::ShowVst()
{
    if ( _vstEffect != NULL && ( _vstEffect->flags & effFlagsHasEditor ) == 0 )
    {
        printf( "This plug does not have an editor!\n" );
        return false;
    }

    Start( DspThread::NormalPriority );
    return true;
}

void VstHost::HideVst()
{
    if ( vstHwnd != NULL )
    {
        EditorProc( vstHwnd, WM_CLOSE, NULL, NULL );
        vstHwnd = NULL;
    }
}

void VstHost::ProcessMidi( VstEvents* events )
{
    vstEffect->dispatcher( _vstEffect, effProcessEvents, 0, 0, events, 0.0f );
}

void VstHost::Process_( DspSignalBus& inputs, DspSignalBus& outputs )
{
    if ( _vstEffect != NULL )
    {
        // convert DSPatch inputs to VST inputs
        for ( unsigned short i = 0; i < _dspInputs.size(); i++ )
        {
            if ( !inputs.GetValue( i, _dspInputs[i] ) )
            {
                memset( _vstInputs[i], 0, _bufferSize * sizeof( float ) );
            }
            else
            {
                memcpy( _vstInputs[i], &_dspInputs[i][0], _bufferSize * sizeof( float ) );
            }
        }

        // process VST
        _vstEffect->processReplacing( _vstEffect, _vstInputs, _vstOutputs, _bufferSize );

        // convert VST outputs to DSPatch outputs
        for ( unsigned short i = 0; i < _dspOutputs.size(); i++ )
        {
            memcpy( &_dspOutputs[i][0], _vstOutputs[i], _bufferSize * sizeof( float ) );
            outputs.SetValue( i, _dspOutputs[i] );
        }
    }
}

void VstHost::_Run()
{
#if _WIN32

    vstEffect = _vstEffect;

    VST_DLGTEMPLATE t;
    t.style = WS_POPUPWINDOW | WS_DLGFRAME | DS_MODALFRAME | DS_CENTER;
    t.cx = 100;
    t.cy = 100;
    DialogBoxIndirectParam( GetModuleHandle( 0 ), &t, 0, (DLGPROC)EditorProc, (LPARAM)_vstEffect );

    vstEffect = NULL;

#elif TARGET_API_MAC_CARBON

    WindowRef window;
    Rect mRect = { 0, 0, 300, 300 };
    OSStatus err = CreateNewWindow( kDocumentWindowClass,
                                    kWindowCloseBoxAttribute | kWindowCompositingAttribute | kWindowAsyncDragAttribute |
                                        kWindowStandardHandlerAttribute,
                                    &mRect, &window );
    if ( err != noErr )
    {
        printf( "HOST> Could not create mac window !\n" );
        return false;
    }
    static EventTypeSpec eventTypes[] = { { kEventClassWindow, kEventWindowClose } };
    InstallWindowEventHandler( window, windowHandler, GetEventTypeCount( eventTypes ), eventTypes, window, NULL );

    printf( "HOST> Open editor...\n" );
    _vstEffect->dispatcher( _vstEffect, effEditOpen, 0, 0, window, 0 );
    ERect* eRect = 0;
    printf( "HOST> Get editor rect..\n" );
    _vstEffect->dispatcher( _vstEffect, effEditGetRect, 0, 0, &eRect, 0 );
    if ( eRect )
    {
        int width = eRect->right - eRect->left;
        int height = eRect->bottom - eRect->top;
        Rect bounds;
        GetWindowBounds( window, kWindowContentRgn, &bounds );
        bounds.right = bounds.left + width;
        bounds.bottom = bounds.top + height;
        SetWindowBounds( window, kWindowContentRgn, &bounds );
    }
    RepositionWindow( window, NULL, kWindowCenterOnMainScreen );
    ShowWindow( window );

    EventLoopTimerRef idleEventLoopTimer;
    InstallEventLoopTimer( GetCurrentEventLoop(), kEventDurationSecond / 25., kEventDurationSecond / 25., idleTimerProc,
                           _vstEffect, &idleEventLoopTimer );

    RunAppModalLoopForWindow( window );
    RemoveEventLoopTimer( idleEventLoopTimer );

    printf( "HOST> Close editor..\n" );
    _vstEffect->dispatcher( _vstEffect, effEditClose, 0, 0, 0, 0 );
    ReleaseWindow( window );

#endif
}
