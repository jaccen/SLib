/*
 *   Copyright (c) 2008-2018 SLIBIO <https://github.com/SLIBIO>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */

#include "slib/core/definition.h"

#if defined(SLIB_UI_IS_ANDROID)

#include "view_android.h"

#include "slib/math/transform2d.h"

namespace slib
{

	SLIB_JNI_BEGIN_CLASS(JAndroidPoint, "android/graphics/Point")
		SLIB_JNI_INT_FIELD(x);
		SLIB_JNI_INT_FIELD(y);
	SLIB_JNI_END_CLASS

	SLIB_JNI_BEGIN_CLASS(JAndroidRect, "android/graphics/Rect")
		SLIB_JNI_INT_FIELD(left);
		SLIB_JNI_INT_FIELD(top);
		SLIB_JNI_INT_FIELD(right);
		SLIB_JNI_INT_FIELD(bottom);
	SLIB_JNI_END_CLASS

	SLIB_JNI_BEGIN_CLASS(JAndroidTouchPoint, "slib/platform/android/ui/view/UiTouchPoint")
		SLIB_JNI_FLOAT_FIELD(x);
		SLIB_JNI_FLOAT_FIELD(y);
		SLIB_JNI_FLOAT_FIELD(pressure);
		SLIB_JNI_INT_FIELD(phase);
		SLIB_JNI_INT_FIELD(pointerId);
	SLIB_JNI_END_CLASS

	void JNICALL _priv_AndroidView_nativeOnDraw(JNIEnv* env, jobject _this, jlong jinstance, jobject jcanvas)
	{
		Ref<Android_ViewInstance> instance = Android_ViewInstance::findInstance(jinstance);
		if (instance.isNotNull()) {
			Ref<Canvas> canvas = GraphicsPlatform::createCanvas(CanvasType::View, jcanvas);
			if (canvas.isNotNull()) {
				instance->onDraw(canvas.get());
			}
		}
	}

	jboolean JNICALL _priv_AndroidView_nativeOnKeyEvent(JNIEnv* env, jobject _this, jlong jinstance, jboolean flagDown, int keycode, jboolean flagControl, jboolean flagShift, jboolean flagAlt, jboolean flagWin, jlong time, jboolean flagDispatchToParent, jboolean flagNotDispatchToChildren)
	{
		Ref<Android_ViewInstance> instance = Android_ViewInstance::findInstance(jinstance);
		if (instance.isNotNull()) {
			UIAction action = flagDown ? UIAction::KeyDown : UIAction::KeyUp;
			sl_uint32 vkey = keycode;
			Keycode key = UIEvent::getKeycodeFromSystemKeycode(keycode);
			Time t;
			t.setMillisecondsCount(time);
			Ref<UIEvent> ev = UIEvent::createKeyEvent(action, key, vkey, t);
			if (ev.isNotNull()) {
				if (flagControl) {
					ev->setControlKey();
				}
				if (flagShift) {
					ev->setShiftKey();
				}
				if (flagAlt) {
					ev->setAltKey();
				}
				if (flagWin) {
					ev->setWindowsKey();
				}
				if (flagDispatchToParent) {
					ev->addFlag(UIEventFlags::DispatchToParent);
				}
				if (flagNotDispatchToChildren) {
					ev->addFlag(UIEventFlags::NotDispatchToChildren);
				}
				instance->onKeyEvent(ev.get());
				if (ev->isPreventedDefault()) {
					return 1;
				}
			}
		}
		return 0;
	}

	jint JNICALL _priv_AndroidView_nativeOnTouchEvent(JNIEnv* env, jobject _this, jlong jinstance, int _action, jobjectArray jpoints, jlong time, jboolean flagDispatchToParent, jboolean flagNotDispatchToChildren)
	{
		Ref<Android_ViewInstance> instance = Android_ViewInstance::findInstance(jinstance);
		if (instance.isNotNull()) {
			UIAction action = (UIAction)_action;
			sl_uint32 nPts = Jni::getArrayLength(jpoints);
			if (nPts > 0) {
				Array<TouchPoint> points = Array<TouchPoint>::create(nPts);
				if (points.isNotNull()) {
					TouchPoint* pts = points.getData();
					for (sl_uint32 i = 0; i < nPts; i++) {
						JniLocal<jobject> jpt = Jni::getObjectArrayElement(jpoints, i);
						if (jpt.isNotNull()) {
							pts[i].point.x = (sl_ui_posf)(JAndroidTouchPoint::x.get(jpt));
							pts[i].point.y = (sl_ui_posf)(JAndroidTouchPoint::y.get(jpt));
							pts[i].pressure = JAndroidTouchPoint::pressure.get(jpt);
							pts[i].phase = (TouchPhase)(JAndroidTouchPoint::phase.get(jpt));
							pts[i].pointerId = JAndroidTouchPoint::pointerId.get(jpt);
						}
					}
					Ref<UIEvent> ev = UIEvent::createTouchEvent(action, points, Time::withMilliseconds(time));
					if (ev.isNotNull()) {
						if (flagDispatchToParent) {
							ev->addFlag(UIEventFlags::DispatchToParent);
						}
						if (flagNotDispatchToChildren) {
							ev->addFlag(UIEventFlags::NotDispatchToChildren);
						}
						instance->onTouchEvent(ev.get());
						return ev->getFlags();
					}
				}
			}
		}
		return 0;
	}

	void JNICALL _priv_AndroidView_nativeOnSetFocus(JNIEnv* env, jobject _this, jlong jinstance)
	{
		Ref<Android_ViewInstance> instance = Android_ViewInstance::findInstance(jinstance);
		if (instance.isNotNull()) {
			instance->onSetFocus();
		}
	}

	void JNICALL _priv_AndroidView_nativeOnClick(JNIEnv* env, jobject _this, jlong jinstance)
	{
		Ref<Android_ViewInstance> instance = Android_ViewInstance::findInstance(jinstance);
		if (instance.isNotNull()) {
			instance->onClick();
		}
	}

	jboolean JNICALL _priv_AndroidView_nativeHitTestTouchEvent(JNIEnv* env, jobject _this, jlong jinstance, int x, int y)
	{
		Ref<Android_ViewInstance> instance = Android_ViewInstance::findInstance(jinstance);
		if (instance.isNotNull()) {
			Ref<View> view = instance->getView();
			if (view.isNotNull()) {
				if (!(view->isEnabled())) {
					return 1;
				}
				Function<sl_bool(const UIPoint&)> hitTestCapture(view->getCapturingChildInstanceEvents());
				if (hitTestCapture.isNotNull()) {
					if (hitTestCapture(UIPoint((sl_ui_pos)x, (sl_ui_pos)y))) {
						return 1;
					}
				}
			}
		}
		return 0;
	}

	void JNICALL _priv_AndroidView_nativeOnSwipe(JNIEnv* env, jobject _this, jlong jinstance, int type)
	{
		Ref<Android_ViewInstance> instance = Android_ViewInstance::findInstance(jinstance);
		if (instance.isNotNull()) {
			instance->onSwipe((GestureType)type);
		}
	}

	SLIB_JNI_BEGIN_CLASS(JAndroidView, "slib/platform/android/ui/view/UiView")

		SLIB_JNI_STATIC_METHOD(getContext, "getContext", "(Landroid/view/View;)Landroid/content/Context;");
		SLIB_JNI_STATIC_METHOD(setInstance, "setInstance", "(Landroid/view/View;J)V");
		SLIB_JNI_STATIC_METHOD(freeView, "freeView", "(Landroid/view/View;)V");

		SLIB_JNI_STATIC_METHOD(createGeneric, "createGeneric", "(Landroid/content/Context;)Landroid/view/View;");
		SLIB_JNI_STATIC_METHOD(createGroup, "createGroup", "(Landroid/content/Context;)Landroid/view/View;");

		SLIB_JNI_STATIC_METHOD(setFocus, "setFocus", "(Landroid/view/View;Z)V");
		SLIB_JNI_STATIC_METHOD(invalidate, "invalidate", "(Landroid/view/View;)V");
		SLIB_JNI_STATIC_METHOD(invalidateRect, "invalidateRect", "(Landroid/view/View;IIII)V");
		SLIB_JNI_STATIC_METHOD(getFrame, "getFrame", "(Landroid/view/View;)Landroid/graphics/Rect;");
		SLIB_JNI_STATIC_METHOD(setFrame, "setFrame", "(Landroid/view/View;IIII)Z");
		SLIB_JNI_STATIC_METHOD(setTransform, "setTransform", "(Landroid/view/View;FFFFFFF)V");
		SLIB_JNI_STATIC_METHOD(isVisible, "isVisible", "(Landroid/view/View;)Z");
		SLIB_JNI_STATIC_METHOD(setVisible, "setVisible", "(Landroid/view/View;Z)V");
		SLIB_JNI_STATIC_METHOD(isEnabled, "isEnabled", "(Landroid/view/View;)Z");
		SLIB_JNI_STATIC_METHOD(setEnabled, "setEnabled", "(Landroid/view/View;Z)V");
		SLIB_JNI_STATIC_METHOD(setAlpha, "setAlpha", "(Landroid/view/View;F)V");
		SLIB_JNI_STATIC_METHOD(setClipping, "setClipping", "(Landroid/view/View;Z)V");
		SLIB_JNI_STATIC_METHOD(setDrawing, "setDrawing", "(Landroid/view/View;Z)V");
		SLIB_JNI_STATIC_METHOD(setLayered, "setLayered", "(Landroid/view/View;)V");
		SLIB_JNI_STATIC_METHOD(convertCoordinateFromScreenToView, "convertCoordinateFromScreenToView", "(Landroid/view/View;II)Landroid/graphics/Point;");
		SLIB_JNI_STATIC_METHOD(convertCoordinateFromViewToScreen, "convertCoordinateFromViewToScreen", "(Landroid/view/View;II)Landroid/graphics/Point;");

		SLIB_JNI_STATIC_METHOD(addChild, "addChild", "(Landroid/view/View;Landroid/view/View;)V");
		SLIB_JNI_STATIC_METHOD(removeChild, "removeChild", "(Landroid/view/View;Landroid/view/View;)V");
		SLIB_JNI_STATIC_METHOD(bringToFront, "bringToFront", "(Landroid/view/View;)V");
		SLIB_JNI_STATIC_METHOD(enableGesture, "enableGesture", "(Landroid/view/View;)V");

		SLIB_JNI_NATIVE(nativeOnDraw, "nativeOnDraw", "(JLslib/platform/android/ui/Graphics;)V", _priv_AndroidView_nativeOnDraw);
		SLIB_JNI_NATIVE(nativeOnKeyEvent, "nativeOnKeyEvent", "(JZIZZZZJZZ)Z", _priv_AndroidView_nativeOnKeyEvent);
		SLIB_JNI_NATIVE(nativeOnTouchEvent, "nativeOnTouchEvent", "(JI[Lslib/platform/android/ui/view/UiTouchPoint;JZZ)I", _priv_AndroidView_nativeOnTouchEvent);
		SLIB_JNI_NATIVE(nativeOnSetFocus, "nativeOnSetFocus", "(J)V", _priv_AndroidView_nativeOnSetFocus);
		SLIB_JNI_NATIVE(nativeOnClick, "nativeOnClick", "(J)V", _priv_AndroidView_nativeOnClick);
		SLIB_JNI_NATIVE(nativeHitTestTouchEvent, "nativeHitTestTouchEvent", "(JII)Z", _priv_AndroidView_nativeHitTestTouchEvent);
		SLIB_JNI_NATIVE(nativeOnSwipe, "nativeOnSwipe", "(JI)V", _priv_AndroidView_nativeOnSwipe);

	SLIB_JNI_END_CLASS

	class _priv_View : public View
	{
		friend class Android_ViewInstance;
	};

/******************************************
			Android_ViewInstance
******************************************/
	Android_ViewInstance::Android_ViewInstance()
	{
	}

	Android_ViewInstance::~Android_ViewInstance()
	{
		jobject jhandle = m_handle.get();
		if (jhandle) {
			UIPlatform::removeViewInstance(jhandle);
			JAndroidView::freeView.call(sl_null, jhandle);
		}
	}

	sl_bool Android_ViewInstance::initialize(jobject jhandle)
	{
		if (jhandle) {
			JniLocal<jobject> jcontext = JAndroidView::getContext.callObject(sl_null, jhandle);
			JniGlobal<jobject> context = jcontext;
			JniGlobal<jobject> handle = jhandle;
			if (context.isNotNull() && handle.isNotNull()) {
				m_context = context;
				m_handle = handle;
				jhandle = handle.get();
				jlong instance = (jlong)(jhandle);
				JAndroidView::setInstance.call(sl_null, jhandle, instance);
				UIPlatform::registerViewInstance(jhandle, this);
				return sl_true;
			}
		}
		return sl_false;
	}

	sl_bool Android_ViewInstance::applyProperties(View* _view, ViewInstance* parent)
	{
		_priv_View* view = (_priv_View*)_view;
		jobject jhandle = m_handle.get();
		if (jhandle) {
			UIRect frame = view->getFrameInInstance();
			JAndroidView::setFrame.callBoolean(sl_null, jhandle, (int)(frame.left), (int)(frame.top), (int)(frame.right), (int)(frame.bottom));
			JAndroidView::setVisible.call(sl_null, jhandle, view->isVisibleInInstance());
			JAndroidView::setEnabled.call(sl_null, jhandle, view->isEnabled());
			sl_real alpha = view->getAlpha();
			JAndroidView::setClipping.call(sl_null, jhandle, view->isClipping());
			JAndroidView::setDrawing.call(sl_null, jhandle, view->isDrawing());
			if (alpha < 0.995f) {
				JAndroidView::setAlpha.call(sl_null, jhandle, alpha);
			}
			if (view->isHardwareLayer()) {
				JAndroidView::setLayered.call(sl_null, jhandle);
			}

			Matrix3 transform = view->getFinalTransformInInstance();
			Vector2 t = Transform2::getTranslationFromMatrix(transform);
			sl_real r = Transform2::getRotationAngleFromMatrix(transform);
			Vector2 s = Transform2::getScaleFromMatrix(transform);
			JAndroidView::setTransform.call(sl_null, jhandle, t.x, t.y, r, s.x, s.y, 0, 0);

			if (parent) {
				jobject jparent = UIPlatform::getViewHandle(parent);
				if (jparent) {
					JAndroidView::addChild.call(sl_null, jparent, jhandle);            
				}            
			}
			return sl_true;
		}
		return sl_false;
	}

	Ref<Android_ViewInstance> Android_ViewInstance::findInstance(jlong jinstance)
	{
		return Ref<Android_ViewInstance>::from(UIPlatform::getViewInstance((jobject)jinstance));
	}

	Ref<View> Android_ViewInstance::findView(jlong jinstance)
	{
		Ref<ViewInstance> instance = UIPlatform::getViewInstance((jobject)jinstance);
		if (instance.isNotNull()) {
			return instance->getView();
		}
		return sl_null;
	}

	jobject Android_ViewInstance::getHandle()
	{
		return m_handle.get();
	}

	jobject Android_ViewInstance::getContext()
	{
		return m_context.get();
	}

	sl_bool Android_ViewInstance::isValid()
	{
		return sl_true;
	}

	void Android_ViewInstance::setFocus(sl_bool flag)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::setFocus.call(sl_null, handle, flag ? sl_true : sl_false);
		}
	}

	void Android_ViewInstance::invalidate()
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::invalidate.call(sl_null, handle);
		}
	}

	void Android_ViewInstance::invalidate(const UIRect& rect)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::invalidateRect.call(sl_null, handle, (int)(rect.left), (int)(rect.top), (int)(rect.right), (int)(rect.bottom));
		}
	}

	UIRect Android_ViewInstance::getFrame()
	{
		jobject handle = m_handle.get();
		if (handle) {
			JniLocal<jobject> jrect = JAndroidView::getFrame.callObject(sl_null, handle);
			if (jrect.isNotNull()) {
				UIRect ret;
				ret.left = (sl_ui_pos)(JAndroidRect::left.get(jrect));
				ret.top = (sl_ui_pos)(JAndroidRect::top.get(jrect));
				ret.right = (sl_ui_pos)(JAndroidRect::right.get(jrect));
				ret.bottom = (sl_ui_pos)(JAndroidRect::bottom.get(jrect));
				ret.fixSizeError();
				return ret;
			}
		}
		return UIRect::zero();
	}

	void Android_ViewInstance::setFrame(const UIRect& frame)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::setFrame.callBoolean(sl_null, handle, (int)(frame.left), (int)(frame.top), (int)(frame.right), (int)(frame.bottom));
		}
	}

	void Android_ViewInstance::setTransform(const Matrix3& transform)
	{
		jobject handle = UIPlatform::getViewHandle(this);
		if (handle) {
			Vector2 t = Transform2::getTranslationFromMatrix(transform);
			sl_real r = Transform2::getRotationAngleFromMatrix(transform);
			Vector2 s = Transform2::getScaleFromMatrix(transform);
			JAndroidView::setTransform.call(sl_null, handle, t.x, t.y, r, s.x, s.y, 0, 0);
		}
	}

	void Android_ViewInstance::setVisible(sl_bool flag)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::setVisible.call(sl_null, handle, flag);
		}
	}

	void Android_ViewInstance::setEnabled(sl_bool flag)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::setEnabled.call(sl_null, handle, flag);
		}
	}

	void Android_ViewInstance::setOpaque(sl_bool flag)
	{
	}

	void Android_ViewInstance::setAlpha(sl_real alpha)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::setAlpha.call(sl_null, handle, (float)alpha);
		}
	}

	void Android_ViewInstance::setClipping(sl_bool flag)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::setClipping.call(sl_null, handle, flag);
		}
	}

	void Android_ViewInstance::setDrawing(sl_bool flag)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::setDrawing.call(sl_null, handle, flag);
		}
	}

	UIPointf Android_ViewInstance::convertCoordinateFromScreenToView(const UIPointf& ptScreen)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JniLocal<jobject> jpt = JAndroidView::convertCoordinateFromScreenToView.callObject(sl_null, handle, 0, 0);
			if (jpt.isNotNull()) {
				UIPointf ret;
				ret.x = ptScreen.x + (sl_ui_pos)(JAndroidPoint::x.get(jpt));
				ret.y = ptScreen.y + (sl_ui_pos)(JAndroidPoint::y.get(jpt));
				return ret;
			}
		}
		return ptScreen;
	}

	UIPointf Android_ViewInstance::convertCoordinateFromViewToScreen(const UIPointf& ptView)
	{
		jobject handle = m_handle.get();
		if (handle) {
			JniLocal<jobject> jpt = JAndroidView::convertCoordinateFromViewToScreen.callObject(sl_null, handle, 0, 0);
			if (jpt.isNotNull()) {
				UIPointf ret;
				ret.x = ptView.x + (sl_ui_pos)(JAndroidPoint::x.get(jpt));
				ret.y = ptView.y + (sl_ui_pos)(JAndroidPoint::y.get(jpt));
				return ret;
			}
		}
		return ptView;
	}

	void Android_ViewInstance::addChildInstance(const Ref<ViewInstance>& _child)
	{
		jobject handle = m_handle.get();
		jobject child = UIPlatform::getViewHandle(_child.get());
		if (handle && child) {
			JAndroidView::addChild.call(sl_null, handle, child);
		}
	}

	void Android_ViewInstance::removeChildInstance(const Ref<ViewInstance>& _child)
	{
		jobject handle = m_handle.get();
		jobject child = UIPlatform::getViewHandle(_child.get());
		if (handle && child) {
			JAndroidView::removeChild.call(sl_null, handle, child);
		}
	}

	void Android_ViewInstance::bringToFront()
	{
		jobject handle = m_handle.get();
		if (handle) {
			JAndroidView::bringToFront.call(sl_null, handle);
		}
	}

/******************************************
				View
******************************************/
	Ref<ViewInstance> View::createGenericInstance(ViewInstance* _parent)
	{
		Ref<Android_ViewInstance> ret;
		Android_ViewInstance* parent = (Android_ViewInstance*)_parent;
		if (parent) {
			JniLocal<jobject> handle;
			if (m_flagCreatingChildInstances) {
				handle = JAndroidView::createGroup.callObject(sl_null, parent->getContext());
			} else {
				handle = JAndroidView::createGeneric.callObject(sl_null, parent->getContext());
			}
			ret = Android_ViewInstance::create<Android_ViewInstance>(this, parent, handle.get());
		}
		return ret;
	}

	/******************************************
					UIPlatform
	******************************************/
	Ref<ViewInstance> UIPlatform::createViewInstance(jobject jhandle)
	{
		Ref<ViewInstance> ret = UIPlatform::_getViewInstance((void*)jhandle);
		if (ret.isNotNull()) {
			return ret;
		}
		return Android_ViewInstance::create<Android_ViewInstance>(jhandle);
	}

	void UIPlatform::registerViewInstance(jobject jhandle, ViewInstance* instance)
	{
		UIPlatform::_registerViewInstance((void*)(jhandle), instance);
	}

	Ref<ViewInstance> UIPlatform::getViewInstance(jobject jhandle)
	{
		return UIPlatform::_getViewInstance((void*)jhandle);
	}

	void UIPlatform::removeViewInstance(jobject jhandle)
	{
		UIPlatform::_removeViewInstance((void*)jhandle);
	}

	jobject UIPlatform::getViewHandle(ViewInstance* _instance)
	{
		Android_ViewInstance* instance = (Android_ViewInstance*)_instance;
		if (instance) {
			return instance->getHandle();
		}
		return 0;
	}

	jobject UIPlatform::getViewHandle(View* view)
	{
		if (view) {
			Ref<Android_ViewInstance> instance = Ref<Android_ViewInstance>::from(view->getViewInstance());
			if (instance.isNotNull()) {
				return instance->getHandle();
			}
		}
		return 0;
	}

	sl_bool GestureDetector::_enableNative(const Ref<View>& view, GestureType type)
	{
		Ref<ViewInstance> _instance = view->getViewInstance();
		Android_ViewInstance* instance = static_cast<Android_ViewInstance*>(_instance.get());
		if (instance) {
			jobject handle = instance->getHandle();
			if (handle) {
				switch (type) {
					case GestureType::SwipeLeft:
					case GestureType::SwipeRight:
					case GestureType::SwipeUp:
					case GestureType::SwipeDown:
						JAndroidView::enableGesture.call(sl_null, handle);
						return sl_true;
					default:
						break;
				}
			}
		}
		return sl_false;
	}

}

#endif
