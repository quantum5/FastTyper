#include <DropTarget.hpp>

#define WM_DROPTEXT (WM_USER+1)
#define WM_DROPFILE (WM_USER+2)

class MyDropTarget : public DropTarget {
    virtual DWORD OnDragEnter(POINTL point, DWORD effect, DWORD state,
                              IDataObject *data);
    virtual DWORD OnDrop(POINTL point, DWORD effect, DWORD state,
                         IDataObject *data);
};
