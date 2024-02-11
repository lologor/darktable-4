/*
    This file is part of darktable,
    Copyright (C) 2011-2020 darktable developers.

    darktable is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    darktable is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with darktable.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "common/darktable.h"
#include "common/debug.h"
#include "common/image_cache.h"
#include "common/iop_group.h"
#include "control/conf.h"
#include "control/control.h"
#include "develop/develop.h"
#include "gui/gtk.h"
#include "libs/lib.h"
#include "libs/lib_api.h"

DT_MODULE(1)

#define DT_IOP_ORDER_INFO (darktable.unmuted & DT_DEBUG_IOPORDER)

#include "modulegroups.h"

typedef struct dt_lib_modulegroups_t
{
  uint32_t current;
  GtkWidget *notebook;
  GtkWidget *text_entry;
  GtkWidget *hbox_search_box;
} dt_lib_modulegroups_t;

/* toggle button callback */
static void _lib_modulegroups_toggle(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data);
/* helper function to update iop module view depending on group */
static void _lib_modulegroups_update_iop_visibility(dt_lib_module_t *self);

/* modulergroups proxy set group function
   \see dt_dev_modulegroups_set()
*/
static void _lib_modulegroups_set(dt_lib_module_t *self, uint32_t group);
/* modulegroups proxy update visibility function
*/
static void _lib_modulegroups_update_visibility_proxy(dt_lib_module_t *self);
/* modulegroups proxy get group function
  \see dt_dev_modulegroups_get()
*/
static uint32_t _lib_modulegroups_get(dt_lib_module_t *self);
/* modulegroups proxy test function.
   tests if iop module group flags matches modulegroup.
*/
static gboolean _lib_modulegroups_test(dt_lib_module_t *self, uint32_t group, uint32_t iop_group);
/* modulegroups proxy switch group function.
   sets the active group which module belongs too.
*/
static void _lib_modulegroups_switch_group(dt_lib_module_t *self, dt_iop_module_t *module);
/* modulergroups proxy search text focus function
   \see dt_dev_modulegroups_search_text_focus()
*/
static void _lib_modulegroups_search_text_focus(dt_lib_module_t *self);

/* hook up with viewmanager view change to initialize modulegroup */
static void _lib_modulegroups_viewchanged_callback(gpointer instance, dt_view_t *old_view,
                                                   dt_view_t *new_view, gpointer data);

const char *name(dt_lib_module_t *self)
{
  return _("modulegroups");
}

const char **views(dt_lib_module_t *self)
{
  static const char *v[] = {"darkroom", NULL};
  return v;
}

uint32_t container(dt_lib_module_t *self)
{
  return DT_UI_CONTAINER_PANEL_RIGHT_TOP;
}


/* this module should always be shown without expander */
int expandable(dt_lib_module_t *self)
{
  return 0;
}

int position()
{
  return 999;
}

static void _text_entry_changed_callback(GtkEntry *entry, dt_lib_module_t *self)
{
  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;
  const gchar *text_entered = gtk_entry_get_text(entry);
  gtk_widget_set_sensitive(d->notebook, !(text_entered && text_entered[0] != '\0'));
  _lib_modulegroups_update_iop_visibility(self);
}

static gboolean _text_entry_icon_press_callback(GtkEntry *entry, GtkEntryIconPosition icon_pos, GdkEvent *event,
                                                dt_lib_module_t *self)
{
  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;
  gtk_entry_set_text(GTK_ENTRY(d->text_entry), "");
  gtk_widget_set_sensitive(d->notebook, TRUE);
  return TRUE;
}

static gboolean _text_entry_key_press_callback(GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
  if(event->keyval == GDK_KEY_Escape)
  {
    dt_lib_module_t *self = (dt_lib_module_t *)user_data;
    dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;
    gtk_entry_set_text(GTK_ENTRY(widget), "");
    gtk_widget_grab_focus(dt_ui_center(darktable.gui->ui));
    gtk_widget_set_sensitive(d->notebook, TRUE);
    return TRUE;
  }

  return FALSE;
}

void view_leave(dt_lib_module_t *self, dt_view_t *old_view, dt_view_t *new_view)
{
  if(!strcmp(old_view->module_name, "darkroom"))
  {
    //dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;
    // FIXME: disconnect actions
  }
}

void view_enter(dt_lib_module_t *self, dt_view_t *old_view, dt_view_t *new_view)
{
  if(!strcmp(new_view->module_name, "darkroom"))
  {
    //dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;
    // FIXME: connect actions
  }
}

void gui_init(dt_lib_module_t *self)
{
  /* initialize ui widgets */
  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)g_malloc0(sizeof(dt_lib_modulegroups_t));
  self->data = (void *)d;

  self->widget = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
  dt_gui_add_help_link(self->widget, dt_get_help_url(self->plugin_name));
  gtk_widget_set_name(self->widget, "modules-tabs");

  /* search box */
  d->hbox_search_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
  d->text_entry = gtk_entry_new();
  gtk_entry_set_placeholder_text(GTK_ENTRY(d->text_entry), _("Search a module…"));
  gtk_widget_add_events(d->text_entry, GDK_FOCUS_CHANGE_MASK);
  gtk_widget_add_events(d->text_entry, GDK_KEY_PRESS_MASK);
  g_signal_connect(G_OBJECT(d->text_entry), "changed", G_CALLBACK(_text_entry_changed_callback), self);
  g_signal_connect(G_OBJECT(d->text_entry), "icon-press", G_CALLBACK(_text_entry_icon_press_callback), self);
  g_signal_connect(G_OBJECT(d->text_entry), "key-press-event", G_CALLBACK(_text_entry_key_press_callback), self);
  gtk_box_pack_start(GTK_BOX(d->hbox_search_box), d->text_entry, TRUE, TRUE, 0);
  gtk_entry_set_width_chars(GTK_ENTRY(d->text_entry), 0);
  gtk_entry_set_icon_from_icon_name(GTK_ENTRY(d->text_entry), GTK_ENTRY_ICON_SECONDARY, "edit-clear");
  gtk_entry_set_icon_tooltip_text(GTK_ENTRY(d->text_entry), GTK_ENTRY_ICON_SECONDARY, _("clear text"));
  gtk_widget_set_name(GTK_WIDGET(d->hbox_search_box), "search-box");
  gtk_box_pack_start(GTK_BOX(self->widget), d->hbox_search_box, TRUE, TRUE, 0);

  /* Tabs */
  d->notebook = GTK_WIDGET(gtk_notebook_new());
  char *labels[DT_MODULEGROUP_SIZE] = { _("Enabled"),  _("Optics"), _("Grading"),
                _("Corrections"), _("Effects"),  _("Scan") };
  char *tooltips[DT_MODULEGROUP_SIZE] = { _("Enabled"),  _("Grading"), _("Effects"),
                _("Effects"), _("Effects"),  _("Effects") };

  for(int i = 0; i < DT_MODULEGROUP_SIZE; i++)
  {
    GtkWidget *label = gtk_label_new(labels[i]);
    gtk_widget_set_tooltip_text(label, tooltips[i]);
    GtkWidget *page = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_notebook_append_page(GTK_NOTEBOOK(d->notebook), page, label);
  }
  gtk_notebook_popup_enable(GTK_NOTEBOOK(d->notebook));
  gtk_notebook_set_scrollable(GTK_NOTEBOOK(d->notebook), TRUE);
  g_signal_connect(G_OBJECT(d->notebook), "switch_page", G_CALLBACK(_lib_modulegroups_toggle), self);

  gtk_box_pack_start(GTK_BOX(self->widget), GTK_WIDGET(d->notebook), TRUE, TRUE, 0);

  if(d->current == DT_MODULEGROUP_NONE) _lib_modulegroups_update_iop_visibility(self);
  gtk_widget_show_all(self->widget);

  /*
   * set the proxy functions
   */
  darktable.develop->proxy.modulegroups.module = self;
  darktable.develop->proxy.modulegroups.set = _lib_modulegroups_set;
  darktable.develop->proxy.modulegroups.update_visibility = _lib_modulegroups_update_visibility_proxy;
  darktable.develop->proxy.modulegroups.get = _lib_modulegroups_get;
  darktable.develop->proxy.modulegroups.test = _lib_modulegroups_test;
  darktable.develop->proxy.modulegroups.switch_group = _lib_modulegroups_switch_group;
  darktable.develop->proxy.modulegroups.search_text_focus = _lib_modulegroups_search_text_focus;

  /* let's connect to view changed signal to set default group */
  dt_control_signal_connect(darktable.signals, DT_SIGNAL_VIEWMANAGER_VIEW_CHANGED,
                            G_CALLBACK(_lib_modulegroups_viewchanged_callback), self);
}

void gui_cleanup(dt_lib_module_t *self)
{
  // dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;
  // FIXME : dt_gui_key_accel_block_on_focus_disconnect(d->text_entry);

  /* let's not listen to signals anymore.. */
  dt_control_signal_disconnect(darktable.signals, G_CALLBACK(_lib_modulegroups_viewchanged_callback), self);

  darktable.develop->proxy.modulegroups.module = NULL;
  darktable.develop->proxy.modulegroups.set = NULL;
  darktable.develop->proxy.modulegroups.get = NULL;
  darktable.develop->proxy.modulegroups.test = NULL;
  darktable.develop->proxy.modulegroups.switch_group = NULL;

  g_free(self->data);
  self->data = NULL;
}

static void _lib_modulegroups_viewchanged_callback(gpointer instance, dt_view_t *old_view,
                                                   dt_view_t *new_view, gpointer data)
{
}

static gboolean _lib_modulegroups_test_internal(dt_lib_module_t *self, uint32_t group, uint32_t iop_group)
{
  if(iop_group & IOP_SPECIAL_GROUP_ACTIVE_PIPE && group == DT_MODULEGROUP_ACTIVE_PIPE)
    return TRUE;
  else if(iop_group & IOP_GROUP_BASIC && group == DT_MODULEGROUP_BASIC)
    return TRUE;
  else if(iop_group & IOP_GROUP_TONE && group == DT_MODULEGROUP_TONE)
    return TRUE;
  else if(iop_group & IOP_GROUP_COLOR && group == DT_MODULEGROUP_COLOR)
    return TRUE;
  else if(iop_group & IOP_GROUP_CORRECT && group == DT_MODULEGROUP_CORRECT)
    return TRUE;
  else if(iop_group & IOP_GROUP_EFFECT && group == DT_MODULEGROUP_EFFECT)
    return TRUE;
  return FALSE;
}

static gboolean _lib_modulegroups_test(dt_lib_module_t *self, uint32_t group, uint32_t iop_group)
{
  return _lib_modulegroups_test_internal(self, group, iop_group);
}

static void _lib_modulegroups_update_iop_visibility(dt_lib_module_t *self)
{
  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;
  const gchar *text_entered = gtk_entry_get_text(GTK_ENTRY(d->text_entry));

  if (DT_IOP_ORDER_INFO)
    fprintf(stderr,"\n^^^^^ modulegroups");

  GList *modules = darktable.develop->iop;
  if(modules)
  {
    /*
     * iterate over iop modules and do various test to
     * detect if the modules should be shown or not.
     */
    do
    {
      dt_iop_module_t *module = (dt_iop_module_t *)modules->data;
      GtkWidget *w = module->expander;

      if ((DT_IOP_ORDER_INFO) && (module->enabled))
      {
        fprintf(stderr,"\n%20s %d",module->op, module->iop_order);
        if(dt_iop_is_hidden(module)) fprintf(stderr,", hidden");
      }

      /* skip modules without an gui */
      if(dt_iop_is_hidden(module)) continue;

      // if there's some search text show matching modules only
      if(text_entered && text_entered[0] != '\0')
      {
        /* don't show deprecated ones unless they are enabled */
        if(module->flags() & IOP_FLAGS_DEPRECATED && !(module->enabled))
        {
          if(darktable.develop->gui_module == module) dt_iop_request_focus(NULL);
          if(w) gtk_widget_hide(w);
        }
        else
        {
          const int is_match = (g_strstr_len(g_utf8_casefold(dt_iop_get_localized_name(module->op), -1), -1,
                                             g_utf8_casefold(text_entered, -1))
                                != NULL);

          if(is_match)
            gtk_widget_show(w);
          else
            gtk_widget_hide(w);
        }
        continue;
      }

      /* lets show/hide modules dependent on current group*/
      switch(d->current)
      {
        case DT_MODULEGROUP_ACTIVE_PIPE:
        {
          if(module->enabled)
          {
            if(w) gtk_widget_show(w);
          }
          else
          {
            if(darktable.develop->gui_module == module) dt_iop_request_focus(NULL);
            if(w) gtk_widget_hide(w);
          }
        }
        break;

        case DT_MODULEGROUP_NONE:
        {
          /* show all except deprecated ones - in case of deprecated, still show it if enabled*/
          if(!(module->flags() & IOP_FLAGS_DEPRECATED) || module->enabled)
          {
            if(w) gtk_widget_show(w);
          }
          else
          {
            if(darktable.develop->gui_module == module) dt_iop_request_focus(NULL);
            if(w) gtk_widget_hide(w);
          }
        }
        break;

        default:
        {
          if(_lib_modulegroups_test_internal(self, d->current, dt_iop_get_group(module))
             && (!(module->flags() & IOP_FLAGS_DEPRECATED) || module->enabled))
          {
            if(w) gtk_widget_show(w);
          }
          else
          {
            if(darktable.develop->gui_module == module) dt_iop_request_focus(NULL);
            if(w) gtk_widget_hide(w);
          }
        }
      }
    } while((modules = g_list_next(modules)) != NULL);
  }
  if (DT_IOP_ORDER_INFO) fprintf(stderr,"\nvvvvv\n");
  // now that visibility has been updated set multi-show
  dt_dev_modules_update_multishow(darktable.develop);
}

static void _lib_modulegroups_toggle(GtkNotebook *notebook, GtkWidget *page, guint page_num, gpointer user_data)
{
  dt_lib_module_t *self = (dt_lib_module_t *)user_data;
  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;

  if(d->current == page_num)
    return; // nothing to do
  else
    d->current = page_num;

  /* clear search text */
  g_signal_handlers_block_matched(d->text_entry, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, _text_entry_changed_callback, NULL);
  gtk_entry_set_text(GTK_ENTRY(d->text_entry), "");
  g_signal_handlers_unblock_matched(d->text_entry, G_SIGNAL_MATCH_FUNC, 0, 0, NULL, _text_entry_changed_callback, NULL);

  /* update visibility */
  _lib_modulegroups_update_iop_visibility(self);
}

typedef struct _set_gui_thread_t
{
  dt_lib_module_t *self;
  uint32_t group;
} _set_gui_thread_t;

static gboolean _lib_modulegroups_set_gui_thread(gpointer user_data)
{
  _set_gui_thread_t *params = (_set_gui_thread_t *)user_data;

  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)params->self->data;

  /* set current group and update visibility */
  if(params->group < DT_MODULEGROUP_SIZE && GTK_IS_NOTEBOOK(d->notebook))
  {
    d->current = params->group;
    gtk_notebook_set_current_page(GTK_NOTEBOOK(d->notebook), params->group);
  }

  _lib_modulegroups_update_iop_visibility(params->self);
  free(params);
  return FALSE;
}

static gboolean _lib_modulegroups_upd_gui_thread(gpointer user_data)
{
  _set_gui_thread_t *params = (_set_gui_thread_t *)user_data;

  _lib_modulegroups_update_iop_visibility(params->self);

  free(params);
  return FALSE;
}

static gboolean _lib_modulegroups_search_text_focus_gui_thread(gpointer user_data)
{
  _set_gui_thread_t *params = (_set_gui_thread_t *)user_data;

  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)params->self->data;

  if(GTK_IS_ENTRY(d->text_entry))
  {
    if(!gtk_widget_is_visible(GTK_WIDGET(d->hbox_search_box))) gtk_widget_show(GTK_WIDGET(d->hbox_search_box));
    gtk_widget_grab_focus(GTK_WIDGET(d->text_entry));
  }

  free(params);
  return FALSE;
}

/* this is a proxy function so it might be called from another thread */
static void _lib_modulegroups_set(dt_lib_module_t *self, uint32_t group)
{
  _set_gui_thread_t *params = (_set_gui_thread_t *)malloc(sizeof(_set_gui_thread_t));
  if(!params) return;
  params->self = self;
  params->group = group;
  g_main_context_invoke(NULL, _lib_modulegroups_set_gui_thread, params);
}

/* this is a proxy function so it might be called from another thread */
static void _lib_modulegroups_update_visibility_proxy(dt_lib_module_t *self)
{
  _set_gui_thread_t *params = (_set_gui_thread_t *)malloc(sizeof(_set_gui_thread_t));
  if(!params) return;
  params->self = self;
  g_main_context_invoke(NULL, _lib_modulegroups_upd_gui_thread, params);
}

/* this is a proxy function so it might be called from another thread */
static void _lib_modulegroups_search_text_focus(dt_lib_module_t *self)
{
  _set_gui_thread_t *params = (_set_gui_thread_t *)malloc(sizeof(_set_gui_thread_t));
  if(!params) return;
  params->self = self;
  params->group = 0;
  g_main_context_invoke(NULL, _lib_modulegroups_search_text_focus_gui_thread, params);
}

static void _lib_modulegroups_switch_group(dt_lib_module_t *self, dt_iop_module_t *module)
{
  /* lets find the group which is not favorite/active pipe */
  for(int k = DT_MODULEGROUP_BASIC; k < DT_MODULEGROUP_SIZE; k++)
  {
    if(_lib_modulegroups_test(self, k, dt_iop_get_group(module)))
    {
      _lib_modulegroups_set(self, k);
      return;
    }
  }
}

static uint32_t _lib_modulegroups_get(dt_lib_module_t *self)
{
  dt_lib_modulegroups_t *d = (dt_lib_modulegroups_t *)self->data;

  for(int k = 0; k < DT_MODULEGROUP_SIZE; k++)
  {
    if (d->current == k)
      return k;
  }
  return DT_MODULEGROUP_NONE;
}

// modelines: These editor modelines have been set for all relevant files by tools/update_modelines.sh
// vim: shiftwidth=2 expandtab tabstop=2 cindent
// kate: tab-indents: off; indent-width 2; replace-tabs on; indent-mode cstyle; remove-trailing-spaces modified;
