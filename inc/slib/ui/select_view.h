#ifndef CHECKHEADER_SLIB_UI_SELECT_VIEW
#define CHECKHEADER_SLIB_UI_SELECT_VIEW

#include "definition.h"

#include "view.h"

SLIB_UI_NAMESPACE_BEGIN

class SelectView;

class SLIB_EXPORT ISelectViewListener
{
public:
	virtual void onSelectItem(SelectView* view, sl_uint32 index) = 0;
	
};

class SLIB_EXPORT SelectView : public View
{
	SLIB_DECLARE_OBJECT
	
public:
	SelectView();
	
public:
	sl_uint32 getItemsCount();
	
	virtual void setItemsCount(sl_uint32 n, sl_bool flagRedraw = sl_true);
	
	void removeAllItems(sl_bool flagRedraw = sl_true);
	
	
	String getItemValue(sl_uint32 index);
	
	virtual void setItemValue(sl_uint32 index, const String& value);
	
	List<String> getValues();
	
	virtual void setValues(const List<String>& values);
	
	
	String getItemTitle(sl_uint32 index);
	
	virtual void setItemTitle(sl_uint32 index, const String& title, sl_bool flagRedraw = sl_true);
	
	List<String> getTitles();
	
	virtual void setTitles(const List<String>& values, sl_bool flagRedraw = sl_true);
	
	
	virtual void selectIndex(sl_uint32 index, sl_bool flagRedraw = sl_true);
	
	virtual void selectValue(const String& value, sl_bool flagRedraw = sl_true);
	
	sl_uint32 getSelectedIndex();
	
	String getSelectedValue();
	
	String getSelectedTitle();

	
	const Size& getIconSize();
	
	virtual void setIconSize(const Size& size, sl_bool flagRedraw = sl_true);
	
	void setIconSize(sl_real width, sl_real height, sl_bool flagRedraw = sl_true);
	
	void setIconSize(sl_real size, sl_bool flagRedraw = sl_true);
	
	sl_real getIconWidth();
	
	void setIconWidth(sl_real width, sl_bool flagRedraw = sl_true);
	
	sl_real getIconHeight();
	
	void setIconHeight(sl_real height, sl_bool flagRedraw = sl_true);
	
	
	Ref<Drawable> getLeftIcon();
	
	virtual void setLeftIcon(const Ref<Drawable>& icon, sl_bool flagRedraw = sl_true);
	
	Ref<Drawable> getRightIcon();
	
	virtual void setRightIcon(const Ref<Drawable>& icon, sl_bool flagRedraw = sl_true);
	
	
	Color getTextColor();
	
	virtual void setTextColor(const Color& color, sl_bool flagRedraw = sl_true);
	
	
public:
	SLIB_PTR_PROPERTY(ISelectViewListener, Listener)
	
public:
	virtual void onSelectItem(sl_uint32 index);
	
protected:
	// override
	void onDraw(Canvas* canvas);
	
	// override
	void onMouseEvent(UIEvent* ev);
	
	// override
	void onMeasureLayout(sl_bool flagHorizontal, sl_bool flagVertical);
	
protected:
	Rectangle getLeftIconRegion();
	
	Rectangle getRightIconRegion();
	
public:
	// override
	Ref<ViewInstance> createNativeWidget(ViewInstance* parent);
	
private:
	void _getSelectedIndex_NW();
	
	void _select_NW(sl_uint32 index);
	
	void _refreshItemsCount_NW();
	
	void _refreshItemsContent_NW();
	
	void _setItemTitle_NW(sl_uint32 index, const String& title);
	
	// override
	void _setFont_NW(const Ref<Font>& font);

protected:
	SafeList<String> m_values;
	SafeList<String> m_titles;
	sl_uint32 m_indexSelected;
	
	Size m_iconSize;
	SafeRef<Drawable> m_leftIcon;
	SafeRef<Drawable> m_rightIcon;
	int m_clickedIconNo;
	
	Color m_textColor;
	
};

SLIB_UI_NAMESPACE_END

#endif