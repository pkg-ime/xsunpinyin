/*
 * Copyright (c) 2010 Mike Qin <mikeandmore@gmail.com>
 *
 * The contents of this file are subject to the terms of either the GNU Lesser
 * General Public License Version 2.1 only ("LGPL") or the Common Development and
 * Distribution License ("CDDL")(collectively, the "License"). You may not use this
 * file except in compliance with the License. You can obtain a copy of the CDDL at
 * http://www.opensource.org/licenses/cddl1.php and a copy of the LGPLv2.1 at
 * http://www.opensource.org/licenses/lgpl-license.php. See the License for the
 * specific language governing permissions and limitations under the License. When
 * distributing the software, include this License Header Notice in each file and
 * include the full text of the License in the License file as well as the
 * following notice:
 *
 * NOTICE PURSUANT TO SECTION 9 OF THE COMMON DEVELOPMENT AND DISTRIBUTION LICENSE
 * (CDDL)
 * For Covered Software in this distribution, this License shall be governed by the
 * laws of the State of California (excluding conflict-of-law provisions).
 * Any litigation relating to this License shall be subject to the jurisdiction of
 * the Federal Courts of the Northern District of California and the state courts
 * of the State of California, with venue lying in Santa Clara County, California.
 *
 * Contributor(s):
 *
 * If you wish your version of this file to be governed by only the CDDL or only
 * the LGPL Version 2.1, indicate your decision by adding "[Contributor]" elects to
 * include this software in this distribution under the [CDDL or LGPL Version 2.1]
 * license." If you don't indicate a single choice of license, a recipient has the
 * option to distribute your version of this file under either the CDDL or the LGPL
 * Version 2.1, or to extend the choice of license to its licensees as provided
 * above. However, if you add LGPL Version 2.1 code and therefore, elected the LGPL
 * Version 2 license, then the option applies only if the new code is made subject
 * to such option by the copyright holder.
 */
#include <locale.h>

#include <sunpinyin.h>

#include "xim.h"
#include "common.h"
#include "settings.h"
#include "sunpinyin_preedit_ui.h"

#define BUF_SIZE 4096

class WindowHandler : public CIMIWinHandler
{
protected:
    virtual void updatePreedit(const IPreeditString* ppd);
    virtual void updateCandidates(const ICandidateList* pcl);
    //virtual void updateStatus(int key, int value);
    virtual void commit(const TWCHAR* str);

private:
    PreeditUI* ui_impl_;

public:

    WindowHandler();
    virtual ~WindowHandler();

    PreeditUI* preedit_ui_impl() { return ui_impl_; }
    void       set_preedit_ui_impl(PreeditUI* ui_impl) {
        ui_impl_ = ui_impl;
    }

    void set_xim_handle(XIMHandle* handle) {
        handle_ = handle;
    }

    bool status() {
        return status_;
    }

    void update_preedit_ui(const IPreeditString* ppd, const char* utf_str);
    void update_candidates_ui(const ICandidateList* pcl, const char* utf_str);

    void pause();
    void move(int x, int y);
    void go_on();
    void reload_ui();

private:
    XIMHandle* handle_;

    char* preedit_str_;
    char* candidate_str_;

    bool status_;
    bool pause_;

    int  x_, y_;
};

WindowHandler::WindowHandler()
{
    preedit_str_ = new char[BUF_SIZE];
    candidate_str_ = new char[BUF_SIZE];
    status_ = false;
    pause_ = false;
    ui_impl_ = NULL;
    handle_ = NULL;
    x_ = y_ = 0;
}

WindowHandler::~WindowHandler()
{
    delete [] preedit_str_;
    delete [] candidate_str_;
}

void
WindowHandler::reload_ui()
{
    ui_impl_->reload();
    ui_impl_->update_preedit_string(preedit_str_);
    ui_impl_->update_candidates_string(candidate_str_);
    if (status_) {
        ui_impl_->show();
        ui_impl_->move(x_, y_);
    } else {
        ui_impl_->hide();
    }
}

void
WindowHandler::pause()
{
    if (status_) {
        ui_impl_->hide();
        status_ = false;
        pause_ = true;
    }
}

void
WindowHandler::move(int x, int y)
{
    x_ = x;
    y_ = y;
    if (ui_impl_) {
        ui_impl_->move(x, y);
    }
}

void
WindowHandler::go_on()
{
    if (!status_ && pause_) {
        ui_impl_->show();
        status_ = true;
        pause_ = false;
    }
}

void
WindowHandler::update_candidates_ui(const ICandidateList* pcl, const char* utf_str)
{
    ui_impl_->update_candidates_string(utf_str);
    if (status_) {
        ui_impl_->show();
    } else {
        ui_impl_->hide();
    }
}

void
WindowHandler::update_preedit_ui(const IPreeditString* ppd, const char* utf_str)
{
    ui_impl_->update_preedit_string(utf_str);
    if (ppd->size() == 0) {
        status_ = false;
    } else {
        status_ = true;
    }
}

void
WindowHandler::updatePreedit(const IPreeditString* ppd)
{
    TIConvSrcPtr src = (TIConvSrcPtr) (ppd->string());
    TWCHAR* front_src = new TWCHAR[BUF_SIZE];
    TWCHAR* end_src = new TWCHAR[BUF_SIZE];

    memset(front_src, 0, BUF_SIZE * sizeof(TWCHAR));
    memset(end_src, 0, BUF_SIZE * sizeof(TWCHAR));

    memcpy(front_src, src, ppd->caret() * sizeof(TWCHAR));
    memcpy(end_src, src + ppd->caret() * sizeof(TWCHAR),
           (ppd->size() - ppd->caret() + 1) * sizeof(TWCHAR));

    memset(preedit_str_, 0, BUF_SIZE);

    WCSTOMBS(preedit_str_, front_src, BUF_SIZE - 1);
    preedit_str_[strlen(preedit_str_)] = '|';
    WCSTOMBS(&preedit_str_[strlen(preedit_str_)], end_src, BUF_SIZE - 1);

    // update within the ui provider
    update_preedit_ui(ppd, preedit_str_);

    delete [] front_src;
    delete [] end_src;
}

void
WindowHandler::updateCandidates(const ICandidateList* pcl)
{
    wstring cand_str;
    for (int i = 0, sz = pcl->size(); i < sz; i++) {
        const TWCHAR* pcand = pcl->candiString(i);
        if (pcand == NULL) break;
        cand_str += (i == 9) ? '0' : TWCHAR('1' + i);
        cand_str += TWCHAR('.');
        cand_str += pcand;
        cand_str += TWCHAR(' ');
    }

    TIConvSrcPtr src = (TIConvSrcPtr)(cand_str.c_str());
    WCSTOMBS(candidate_str_, (const TWCHAR*) src, BUF_SIZE - 1);

    // update within the ui provider
    update_candidates_ui(pcl, candidate_str_);
}

void
WindowHandler::commit(const TWCHAR* str)
{
    char* buf = new char[BUF_SIZE];
    memset(buf, 0, BUF_SIZE);
    WCSTOMBS(buf, str, BUF_SIZE - 1);
    if (handle_ != NULL) {
        xim_commit_preedit(handle_, buf);
    }
    delete [] buf;
}


static PreeditUI* ui_impl = NULL;
static WindowHandler* instance = NULL;
static CIMIView* view = NULL;

__EXPORT_API void
preedit_init()
{
    CSunpinyinSessionFactory& fac = CSunpinyinSessionFactory::getFactory();
    if (settings_get_int(SHUANGPIN)) {
        fac.setPinyinScheme(CSunpinyinSessionFactory::SHUANGPIN);
        // shuangpin schemes
        varchar scheme;
        settings_get(SHUANGPIN_SCHEME, scheme);
        if (strcmp(scheme, "MS2003") == 0) {
            AShuangpinSchemePolicy::instance().setShuangpinType(MS2003);
        } else if (strcmp(scheme, "ABC") == 0) {
            AShuangpinSchemePolicy::instance().setShuangpinType(ABC);
        } else if (strcmp(scheme, "ZiRanMa") == 0) {
            AShuangpinSchemePolicy::instance().setShuangpinType(ZIRANMA);
        } else if (strcmp(scheme, "PinYin++") == 0) {
            AShuangpinSchemePolicy::instance().setShuangpinType(PINYINJIAJIA);
        } else if (strcmp(scheme, "ZiGuang") == 0) {
            AShuangpinSchemePolicy::instance().setShuangpinType(ZIGUANG);
        } else if (strcmp(scheme, "XiaoHe") == 0) {
            AShuangpinSchemePolicy::instance().setShuangpinType(XIAOHE);
        }
    } else {
        fac.setPinyinScheme(CSunpinyinSessionFactory::QUANPIN);
    }
    view = fac.createSession();

    varchar skin_name;
    settings_get(SKIN_NAME, skin_name);
    ui_impl = create_preedit_ui(skin_name);

    instance = new WindowHandler();
    instance->set_preedit_ui_impl(ui_impl);
    view->getIC()->setCharsetLevel(1);// GBK
    view->attachWinHandler(instance);
}

__EXPORT_API void
preedit_finalize(void)
{
    LOG("preedit_finalizing...");
    CSunpinyinSessionFactory& fac = CSunpinyinSessionFactory::getFactory();
    fac.destroySession(view);

    delete ui_impl;
    delete instance;
}

__EXPORT_API void
preedit_reload(void)
{
    // number of candidates
    view->setCandiWindowSize(settings_get_int(CANDIDATES_SIZE));

    // page up/down key
    CHotkeyProfile* prof = view->getHotkeyProfile();
    prof->clear();
    if (settings_get_int(PAGE_MINUS_PLUS)) {
        prof->addPageUpKey(CKeyEvent(IM_VK_MINUS));
        prof->addPageDownKey(CKeyEvent(IM_VK_EQUALS));
    }
    if (settings_get_int(PAGE_COMMA_PERIOD)) {
        prof->addPageUpKey(CKeyEvent(IM_VK_COMMA));
        prof->addPageDownKey(CKeyEvent(IM_VK_PERIOD));
    }
    if (settings_get_int(PAGE_PAREN)) {
        prof->addPageUpKey(CKeyEvent('['));
        prof->addPageDownKey(CKeyEvent(']'));
    }

    // fuzzy segmentation
    bool enable_fuzzy = settings_get_int(FUZZY_SEGMENTATION);
    bool enable_inner = settings_get_int(FUZZY_INNER_SEGMENTATION);
    AQuanpinSchemePolicy::instance().setFuzzySegmentation(enable_fuzzy);
    AQuanpinSchemePolicy::instance().setInnerFuzzySegmentation(enable_inner);

    // cancel last selection on backspace
    view->setCancelOnBackspace(settings_get_int(CANCEL_ON_BACKSPACE));

    // do we need to change the skin?
    varchar skin_name;
    settings_get(SKIN_NAME, skin_name);
    if (ui_impl->name() != skin_name) {
        delete ui_impl;
        ui_impl = create_preedit_ui(skin_name);
        instance->set_preedit_ui_impl(ui_impl);
    }

    instance->reload_ui();
}

__EXPORT_API void
preedit_set_handle(XIMHandle* handle)
{
    instance->set_xim_handle(handle);
}

__EXPORT_API void
preedit_move(int x, int y)
{
    instance->move(x, y);
}

__EXPORT_API void
preedit_pause(void)
{
    instance->pause();
}

__EXPORT_API void
preedit_go_on(void)
{
    instance->go_on();
}

__EXPORT_API void
preedit_on_key(XIMHandle* handle, unsigned int keycode, unsigned int state)
{
    if (keycode < 0x20 && keycode > 0x7E)
        keycode = 0;
    view->onKeyEvent(CKeyEvent(keycode, keycode, state));
}

__EXPORT_API bool
preedit_status(void)
{
    return instance->status();
}

__EXPORT_API void
preedit_set_full(bool full)
{
    view->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLSYMBOL, full);
}

__EXPORT_API void
preedit_set_chinese_punc(bool chn_punc)
{
    view->setStatusAttrValue(CIMIWinHandler::STATUS_ID_FULLPUNC, chn_punc);
}

__EXPORT_API void
preedit_omit_next_punct()
{
    view->getIC()->omitNextPunct();
}

