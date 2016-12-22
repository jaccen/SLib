#include "../../../inc/slib/core/definition.h"

#if defined(SLIB_PLATFORM_IS_IOS)

#include "../../../inc/slib/ui/picker_view.h"

#include "view_ios.h"

@interface _Slib_iOS_PickerView : UIPickerView<UIPickerViewDelegate, UIPickerViewDataSource>
{
	@public sl_uint32 m_selectionBefore;
	
	@public slib::WeakRef<slib::iOS_ViewInstance> m_viewInstance;
	@public slib::WeakRef<slib::PickerView> m_view;
	
	@public UIFont* m_font;
	
}
@end

SLIB_UI_NAMESPACE_BEGIN

class _PickerView : public PickerView
{
public:
	sl_uint32 __getItemsCount()
	{
		return (sl_uint32)(m_titles.getCount());
	}
	
	NSString* __getItemTitle(sl_uint32 row)
	{
		String s = m_titles.getItemValue(row, String::null());
		return Apple::getNSStringFromString(s);
	}
	
	void __onSelectItem(_Slib_iOS_PickerView* v, sl_uint32 row)
	{
		m_indexSelected = row;
		onSelectItem(row);
	}
	
	void __selectItem(_Slib_iOS_PickerView* v, sl_uint32 row)
	{
		dispatch_async(dispatch_get_main_queue(), ^{
			[v selectRow:row inComponent:0 animated:NO];
		});
	}
};

Ref<ViewInstance> PickerView::createNativeWidget(ViewInstance* _parent)
{
	IOS_VIEW_CREATE_INSTANCE_BEGIN
	_Slib_iOS_PickerView* handle = [[_Slib_iOS_PickerView alloc] initWithFrame:frame];
	if (handle != nil) {
		Ref<Font> font = getFont();
		UIFont* hFont = GraphicsPlatform::getUIFont(font.ptr, UIPlatform::getGlobalScaleFactor());
		if (hFont != nil) {
			handle->m_font = hFont;
		}
		handle->m_view = this;
	}
	IOS_VIEW_CREATE_INSTANCE_END
	if (handle != nil) {
		((_PickerView*)this)->__selectItem(handle, m_indexSelected);
	}
	return ret;
}

void PickerView::_getSelectedIndex_NW()
{
}

void PickerView::_select_NW(sl_uint32 index)
{
	UIView* handle = UIPlatform::getViewHandle(this);
	if (handle != nil && [handle isKindOfClass:[_Slib_iOS_PickerView class]]) {
		_Slib_iOS_PickerView* v = (_Slib_iOS_PickerView*)handle;
		((_PickerView*)this)->__selectItem(v, index);
	}
}

void PickerView::_refreshItemsCount_NW()
{
	UIView* handle = UIPlatform::getViewHandle(this);
	if (handle != nil && [handle isKindOfClass:[_Slib_iOS_PickerView class]]) {
		_Slib_iOS_PickerView* v = (_Slib_iOS_PickerView*)handle;
		[v reloadAllComponents];
		((_PickerView*)this)->__selectItem(handle, m_indexSelected);
	}
}

void PickerView::_refreshItemsContent_NW()
{
	UIView* handle = UIPlatform::getViewHandle(this);
	if (handle != nil && [handle isKindOfClass:[_Slib_iOS_PickerView class]]) {
		_Slib_iOS_PickerView* v = (_Slib_iOS_PickerView*)handle;
		[v reloadAllComponents];
		((_PickerView*)this)->__selectItem(handle, m_indexSelected);
	}
}

void PickerView::_setItemTitle_NW(sl_uint32 index, const String& title)
{
	UIView* handle = UIPlatform::getViewHandle(this);
	if (handle != nil && [handle isKindOfClass:[_Slib_iOS_PickerView class]]) {
		_Slib_iOS_PickerView* v = (_Slib_iOS_PickerView*)handle;
		[v reloadComponent:index];
	}
}

void PickerView::_setFont_NW(const Ref<Font>& font)
{
	UIView* handle = UIPlatform::getViewHandle(this);
	if (handle != nil && [handle isKindOfClass:[_Slib_iOS_PickerView class]]) {
		_Slib_iOS_PickerView* v = (_Slib_iOS_PickerView*)handle;
		UIFont* hFont = GraphicsPlatform::getUIFont(font.ptr, UIPlatform::getGlobalScaleFactor());
		if (hFont != nil) {
			v->m_font = hFont;
			[v reloadAllComponents];
		}
	}
}

SLIB_UI_NAMESPACE_END

@implementation _Slib_iOS_PickerView
-(id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame:frame];
	
	if (self != nil) {
	
		self->m_selectionBefore = 0;
		
		self.showsSelectionIndicator = YES;
		self.dataSource = self;
		self.delegate = self;

	}
	return self;
}

- (NSInteger)numberOfComponentsInPickerView:(UIPickerView *)pickerView;
{
	return 1;
}

- (void)pickerView:(UIPickerView *)pickerView didSelectRow:(NSInteger)row inComponent:(NSInteger)component
{
	slib::Ref<slib::PickerView> view = m_view;
	if (view.isNotNull()) {
		((slib::_PickerView*)(view.ptr))->__onSelectItem(self, (sl_uint32)row);
	}
}

- (NSInteger)pickerView:(UIPickerView *)pickerView numberOfRowsInComponent:(NSInteger)component;
{
	slib::Ref<slib::PickerView> view = m_view;
	if (view.isNotNull()) {
		return (NSInteger)(((slib::_PickerView*)(view.ptr))->__getItemsCount());
	}
	return 0;
}

- (UIView *)pickerView:(UIPickerView *)pickerView viewForRow:(NSInteger)row forComponent:(NSInteger)component reusingView:(UIView *)view{
	UILabel* label = (UILabel*)view;
	if (!label){
		label = [[UILabel alloc] init];
		label.font = m_font;
		label.textAlignment = NSTextAlignmentCenter;
	}
	slib::Ref<slib::PickerView> picker = m_view;
	if (picker.isNotNull()) {
		label.text = ((slib::_PickerView*)(picker.ptr))->__getItemTitle((sl_uint32)row);
	}
	return label;
}

- (CGFloat)pickerView:(UIPickerView *)pickerView rowHeightForComponent:(NSInteger)component
{
	UIFont* font = m_font;
	if (font != nil) {
		CGFloat h = font.lineHeight;
		return h;
	} else {
		return 12;
	}
}

@end

#endif
