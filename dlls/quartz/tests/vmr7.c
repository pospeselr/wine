/*
 * Video Mixing Renderer 7 unit tests
 *
 * Copyright 2018 Zebediah Figura
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#define COBJMACROS
#include "dshow.h"
#include "d3d9.h"
#include "vmr9.h"
#include "wine/test.h"

static IBaseFilter *create_vmr7(DWORD mode)
{
    IBaseFilter *filter = NULL;
    IVMRFilterConfig *config;
    HRESULT hr = CoCreateInstance(&CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
            &IID_IBaseFilter, (void **)&filter);
    ok(hr == S_OK, "Got hr %#x.\n", hr);
    if (mode)
    {
        hr = IBaseFilter_QueryInterface(filter, &IID_IVMRFilterConfig, (void **)&config);
        ok(hr == S_OK, "Got hr %#x.\n", hr);
        hr = IVMRFilterConfig_SetRenderingMode(config, mode);
        ok(hr == S_OK, "Got hr %#x.\n", hr);
        IVMRFilterConfig_Release(config);
    }
    return filter;
}

static void test_filter_config(void)
{
    IVMRFilterConfig *config;
    DWORD count, mode;
    HRESULT hr;
    ULONG ref;

    hr = CoCreateInstance(&CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
            &IID_IVMRFilterConfig, (void **)&config);
    ok(hr == S_OK, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_GetRenderingMode(config, &mode);
    ok(hr == S_OK, "Got hr %#x.\n", hr);
    ok(mode == VMRMode_Windowed, "Got mode %#x.\n", mode);

    hr = IVMRFilterConfig_SetRenderingMode(config, VMRMode_Windowed);
    ok(hr == S_OK, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_GetRenderingMode(config, &mode);
    ok(hr == S_OK, "Got hr %#x.\n", hr);
    ok(mode == VMRMode_Windowed, "Got mode %#x.\n", mode);

    hr = IVMRFilterConfig_SetRenderingMode(config, VMRMode_Windowed);
    ok(hr == VFW_E_WRONG_STATE, "Got hr %#x.\n", hr);

    ref = IVMRFilterConfig_Release(config);
    ok(!ref, "Got outstanding refcount %d.\n", ref);

    hr = CoCreateInstance(&CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
            &IID_IVMRFilterConfig, (void **)&config);
    ok(hr == S_OK, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_SetRenderingMode(config, VMRMode_Windowless);
    ok(hr == S_OK, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_GetRenderingMode(config, &mode);
    ok(hr == S_OK, "Got hr %#x.\n", hr);
    ok(mode == VMRMode_Windowless, "Got mode %#x.\n", mode);

    hr = IVMRFilterConfig_SetRenderingMode(config, VMRMode_Windowed);
    ok(hr == VFW_E_WRONG_STATE, "Got hr %#x.\n", hr);

    ref = IVMRFilterConfig_Release(config);
    ok(!ref, "Got outstanding refcount %d.\n", ref);

    hr = CoCreateInstance(&CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
            &IID_IVMRFilterConfig, (void **)&config);
    ok(hr == S_OK, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_SetRenderingMode(config, VMRMode_Renderless);
    ok(hr == S_OK, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_GetRenderingMode(config, &mode);
    ok(hr == S_OK, "Got hr %#x.\n", hr);
    ok(mode == VMRMode_Renderless, "Got mode %#x.\n", mode);

    hr = IVMRFilterConfig_SetRenderingMode(config, VMRMode_Windowless);
    ok(hr == VFW_E_WRONG_STATE, "Got hr %#x.\n", hr);

    ref = IVMRFilterConfig_Release(config);
    ok(!ref, "Got outstanding refcount %d.\n", ref);

    hr = CoCreateInstance(&CLSID_VideoMixingRenderer, NULL, CLSCTX_INPROC_SERVER,
            &IID_IVMRFilterConfig, (void **)&config);
    ok(hr == S_OK, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_GetNumberOfStreams(config, &count);
    todo_wine ok(hr == VFW_E_VMR_NOT_IN_MIXER_MODE, "Got hr %#x.\n", hr);

    hr = IVMRFilterConfig_SetNumberOfStreams(config, 3);
    if (hr != VFW_E_DDRAW_CAPS_NOT_SUITABLE)
    {
        todo_wine ok(hr == S_OK, "Got hr %#x.\n", hr);

        hr = IVMRFilterConfig_GetNumberOfStreams(config, &count);
        todo_wine {
            ok(hr == S_OK, "Got hr %#x.\n", hr);
            ok(count == 3, "Got count %u.\n", count);
        }

        hr = IVMRFilterConfig_GetRenderingMode(config, &mode);
        ok(hr == S_OK, "Got hr %#x.\n", hr);
        ok(mode == VMRMode_Windowed, "Got mode %#x.\n", mode);

        /* Despite MSDN, you can still change the rendering mode after setting the
         * stream count. */
        hr = IVMRFilterConfig_SetRenderingMode(config, VMRMode_Windowless);
        ok(hr == S_OK, "Got hr %#x.\n", hr);

        hr = IVMRFilterConfig_GetRenderingMode(config, &mode);
        ok(hr == S_OK, "Got hr %#x.\n", hr);
        ok(mode == VMRMode_Windowless, "Got mode %#x.\n", mode);

        hr = IVMRFilterConfig_GetNumberOfStreams(config, &count);
        todo_wine {
            ok(hr == S_OK, "Got hr %#x.\n", hr);
            ok(count == 3, "Got count %u.\n", count);
        }
    }
    else
        skip("Mixing mode is not supported.\n");

    ref = IVMRFilterConfig_Release(config);
    ok(!ref, "Got outstanding refcount %d.\n", ref);
}

#define check_interface(a, b, c) check_interface_(__LINE__, a, b, c)
static void check_interface_(unsigned int line, void *iface_ptr, REFIID iid, BOOL supported)
{
    IUnknown *iface = iface_ptr;
    HRESULT hr, expected_hr;
    IUnknown *unk;

    expected_hr = supported ? S_OK : E_NOINTERFACE;

    hr = IUnknown_QueryInterface(iface, iid, (void **)&unk);
    ok_(__FILE__, line)(hr == expected_hr, "Got hr %#x, expected %#x.\n", hr, expected_hr);
    if (SUCCEEDED(hr))
        IUnknown_Release(unk);
}

static void test_interfaces(void)
{
    IBaseFilter *filter = create_vmr7(0);
    ULONG ref;

    check_interface(filter, &IID_IAMCertifiedOutputProtection, TRUE);
    check_interface(filter, &IID_IAMFilterMiscFlags, TRUE);
    check_interface(filter, &IID_IBaseFilter, TRUE);
    check_interface(filter, &IID_IBasicVideo, TRUE);
    todo_wine check_interface(filter, &IID_IBasicVideo2, TRUE);
    todo_wine check_interface(filter, &IID_IKsPropertySet, TRUE);
    check_interface(filter, &IID_IMediaFilter, TRUE);
    check_interface(filter, &IID_IMediaPosition, TRUE);
    check_interface(filter, &IID_IMediaSeeking, TRUE);
    check_interface(filter, &IID_IPersist, TRUE);
    check_interface(filter, &IID_IQualityControl, TRUE);
    todo_wine check_interface(filter, &IID_IQualProp, TRUE);
    check_interface(filter, &IID_IUnknown, TRUE);
    check_interface(filter, &IID_IVideoWindow, TRUE);
    todo_wine check_interface(filter, &IID_IVMRAspectRatioControl, TRUE);
    todo_wine check_interface(filter, &IID_IVMRDeinterlaceControl, TRUE);
    check_interface(filter, &IID_IVMRFilterConfig, TRUE);
    todo_wine check_interface(filter, &IID_IVMRMixerBitmap, TRUE);
    check_interface(filter, &IID_IVMRMonitorConfig, TRUE);

    check_interface(filter, &IID_IBasicAudio, FALSE);
    check_interface(filter, &IID_IDirectDrawVideo, FALSE);
    check_interface(filter, &IID_IPersistPropertyBag, FALSE);
    check_interface(filter, &IID_IPin, FALSE);
    check_interface(filter, &IID_IReferenceClock, FALSE);
    check_interface(filter, &IID_IVMRAspectRatioControl9, FALSE);
    check_interface(filter, &IID_IVMRDeinterlaceControl9, FALSE);
    todo_wine check_interface(filter, &IID_IVMRFilterConfig9, FALSE);
    check_interface(filter, &IID_IVMRMixerBitmap9, FALSE);
    check_interface(filter, &IID_IVMRMixerControl, FALSE);
    check_interface(filter, &IID_IVMRMixerControl9, FALSE);
    todo_wine check_interface(filter, &IID_IVMRMonitorConfig9, FALSE);
    check_interface(filter, &IID_IVMRSurfaceAllocatorNotify, FALSE);
    check_interface(filter, &IID_IVMRSurfaceAllocatorNotify9, FALSE);
    check_interface(filter, &IID_IVMRWindowlessControl, FALSE);
    check_interface(filter, &IID_IVMRWindowlessControl9, FALSE);

    IBaseFilter_Release(filter);
    filter = create_vmr7(VMRMode_Windowless);

    check_interface(filter, &IID_IVMRMonitorConfig, TRUE);
    check_interface(filter, &IID_IVMRWindowlessControl, TRUE);

    todo_wine check_interface(filter, &IID_IBasicVideo, FALSE);
    check_interface(filter, &IID_IBasicVideo2, FALSE);
    todo_wine check_interface(filter, &IID_IVideoWindow, FALSE);
    check_interface(filter, &IID_IVMRSurfaceAllocatorNotify, FALSE);
    check_interface(filter, &IID_IVMRSurfaceAllocatorNotify9, FALSE);
    check_interface(filter, &IID_IVMRMixerControl, FALSE);
    check_interface(filter, &IID_IVMRMixerControl9, FALSE);
    todo_wine check_interface(filter, &IID_IVMRMonitorConfig9, FALSE);
    todo_wine check_interface(filter, &IID_IVMRWindowlessControl9, FALSE);

    IBaseFilter_Release(filter);
    filter = create_vmr7(VMRMode_Renderless);

    check_interface(filter, &IID_IVMRSurfaceAllocatorNotify, TRUE);

    todo_wine check_interface(filter, &IID_IBasicVideo, FALSE);
    check_interface(filter, &IID_IBasicVideo2, FALSE);
    todo_wine check_interface(filter, &IID_IVideoWindow, FALSE);
    check_interface(filter, &IID_IVMRMixerControl, FALSE);
    todo_wine check_interface(filter, &IID_IVMRMonitorConfig, FALSE);
    todo_wine check_interface(filter, &IID_IVMRMonitorConfig9, FALSE);
    todo_wine check_interface(filter, &IID_IVMRSurfaceAllocatorNotify9, FALSE);
    check_interface(filter, &IID_IVMRWindowlessControl, FALSE);
    check_interface(filter, &IID_IVMRWindowlessControl9, FALSE);

    ref = IBaseFilter_Release(filter);
    ok(!ref, "Got outstanding refcount %d.\n", ref);
}

START_TEST(vmr7)
{
    CoInitialize(NULL);

    test_filter_config();
    test_interfaces();

    CoUninitialize();
}
