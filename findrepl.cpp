#include "bsppch.hpp"
#pragma hdrstop

#include "bspx.hpp"
#include "qtype.hpp"
#include "bsp.rh"
#include "bspx2.hpp"

class TBSPFindReplaceDialog : public TFindReplaceDialog {
	public:

		TBSPFindReplaceDialog(TWindow* parent,
			TData& data, TResId templateId = 0,
			const char far* title = 0,
			TModule* module = 0);

		CmReplace();
};

DEFINE_RESPONSE_TABLE1(TBSPFindReplaceDialog, TFindReplaceDialog)
	EV_CM_REPLACE,
//  EV_WM_ERASEBKGND,
END_RESPONSE_TABLE;

TBSPFindReplaceDialog::TBSPFindReplaceDialog(TWindow* parent,
	TData& data, TResId templateId = 0,
	const char far* title = 0,
	TModule* module = 0)
	:
		TFindReplaceDialog(parent,data,templateId,title,module)
{
};

void TBSPFindReplaceDialog::CmReplace() {
// handle it...

};
