// Copyright (C) 2013 Graeme Gott <graeme@gottcode.org>
//
// This library is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this library.  If not, see <http://www.gnu.org/licenses/>.


#include "configuration_dialog.hpp"

#include "applications_page.hpp"
#include "favorites_page.hpp"
#include "icon_size.hpp"
#include "launcher.hpp"
#include "launcher_view.hpp"
#include "menu.hpp"
#include "panel_plugin.hpp"
#include "section_button.hpp"

extern "C"
{
#include <exo/exo.h>
#include <libxfce4panel/libxfce4panel.h>
#include <libxfce4ui/libxfce4ui.h>
}

using namespace WhiskerMenu;

//-----------------------------------------------------------------------------

static void whiskermenu_config_dialog_delete(ConfigurationDialog* dialog)
{
	delete dialog;
	dialog = NULL;
}

//-----------------------------------------------------------------------------

ConfigurationDialog::ConfigurationDialog(PanelPlugin* plugin) :
	m_plugin(plugin)
{
	// Create dialog window
	GtkWindow* window = NULL;
	GtkWidget* toplevel = gtk_widget_get_toplevel(m_plugin->get_button());
	if (gtk_widget_is_toplevel(toplevel))
	{
		window = GTK_WINDOW(toplevel);
	}
	m_window = xfce_titled_dialog_new_with_buttons(_("Whisker Menu"), window, GTK_DIALOG_NO_SEPARATOR, GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
	gtk_window_set_icon_name(GTK_WINDOW(m_window), GTK_STOCK_PROPERTIES);
	gtk_window_set_position(GTK_WINDOW(m_window), GTK_WIN_POS_CENTER);
	g_signal_connect(m_window, "response", G_CALLBACK(ConfigurationDialog::response_slot), this);
	g_signal_connect_swapped(m_window, "destroy", G_CALLBACK(whiskermenu_config_dialog_delete), this);

	// Create tabs
	GtkNotebook* notebook = GTK_NOTEBOOK(gtk_notebook_new());
	gtk_notebook_append_page(notebook, init_appearance_tab(), gtk_label_new_with_mnemonic("_Appearance"));
	gtk_notebook_append_page(notebook, init_panel_button_tab(), gtk_label_new_with_mnemonic("_Panel Button"));
	gtk_notebook_append_page(notebook, init_behavior_tab(), gtk_label_new_with_mnemonic("_Behavior"));
	gtk_notebook_append_page(notebook, init_commands_tab(), gtk_label_new_with_mnemonic("_Commands"));

	// Add tabs to dialog
	GtkBox* vbox = GTK_BOX(gtk_vbox_new(false, 8));
	gtk_container_set_border_width(GTK_CONTAINER(vbox), 6);
	gtk_box_pack_start(vbox, GTK_WIDGET(notebook), true, true, 0);
	GtkBox* contents = GTK_BOX(gtk_dialog_get_content_area(GTK_DIALOG(m_window)));
	gtk_box_pack_start(contents, GTK_WIDGET(vbox), true, true, 0);

	// Show GTK window
	gtk_widget_show_all(m_window);

	m_plugin->set_configure_enabled(false);
}

//-----------------------------------------------------------------------------

ConfigurationDialog::~ConfigurationDialog()
{
	m_plugin->set_configure_enabled(true);
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::choose_icon()
{
	GtkWidget* chooser = exo_icon_chooser_dialog_new(_("Select An Icon"),
			GTK_WINDOW(m_window),
			GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
			NULL);

	gtk_dialog_set_default_response(GTK_DIALOG(chooser), GTK_RESPONSE_ACCEPT);
	gtk_dialog_set_alternative_button_order(GTK_DIALOG(chooser),
			GTK_RESPONSE_ACCEPT,
			GTK_RESPONSE_CANCEL, -1);
	exo_icon_chooser_dialog_set_icon(EXO_ICON_CHOOSER_DIALOG(chooser), m_plugin->get_button_icon_name().c_str());

	if (gtk_dialog_run(GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT)
	{
		gchar* icon = exo_icon_chooser_dialog_get_icon(EXO_ICON_CHOOSER_DIALOG(chooser));
		xfce_panel_image_set_from_source(XFCE_PANEL_IMAGE(m_icon), icon);
		m_plugin->set_button_icon_name(icon);
		g_free(icon);
	}

	gtk_widget_destroy(chooser);
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::category_icon_size_changed(GtkComboBox* combo)
{
	SectionButton::set_icon_size(gtk_combo_box_get_active(combo));
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::item_icon_size_changed(GtkComboBox* combo)
{
	LauncherView::set_icon_size(gtk_combo_box_get_active(combo));
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::style_changed(GtkComboBox* combo)
{
	m_plugin->set_button_style(PanelPlugin::ButtonStyle(gtk_combo_box_get_active(combo) + 1));
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::title_changed()
{
	const gchar* text = gtk_entry_get_text(GTK_ENTRY(m_title));
	m_plugin->set_button_title(text ? text : "");
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_hover_switch_category(GtkToggleButton* button)
{
	SectionButton::set_hover_activate(gtk_toggle_button_get_active(button));
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_show_name(GtkToggleButton* button)
{
	Launcher::set_show_name(gtk_toggle_button_get_active(button));
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_show_description(GtkToggleButton* button)
{
	Launcher::set_show_description(gtk_toggle_button_get_active(button));
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_load_hierarchy(GtkToggleButton* button)
{
	ApplicationsPage::set_load_hierarchy(gtk_toggle_button_get_active(button));
	m_plugin->reload();
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::toggle_remember_favorites(GtkToggleButton* button)
{
	FavoritesPage::set_remember_favorites(gtk_toggle_button_get_active(button));
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::settings_command_changed()
{
	const gchar* text = gtk_entry_get_text(GTK_ENTRY(m_settings_command));
	Menu::set_settings_command(text ? text : "");
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::lockscreen_command_changed()
{
	const gchar* text = gtk_entry_get_text(GTK_ENTRY(m_lockscreen_command));
	Menu::set_lockscreen_command(text ? text : "");
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::logout_command_changed()
{
	const gchar* text = gtk_entry_get_text(GTK_ENTRY(m_logout_command));
	Menu::set_logout_command(text ? text : "");
}

//-----------------------------------------------------------------------------

void ConfigurationDialog::response(int response_id)
{
	if ((m_plugin->get_button_style() == PanelPlugin::ShowText) && m_plugin->get_button_title().empty())
	{
		m_plugin->set_button_title(PanelPlugin::get_button_title_default());
	}

	if (response_id == GTK_RESPONSE_CLOSE)
	{
		gtk_widget_destroy(m_window);
	}
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_appearance_tab()
{
	// Create size group for labels
	GtkSizeGroup* label_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	// Create appearance section
	GtkWidget* page = gtk_alignment_new(0, 0, 1, 0);
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);
	GtkBox* appearance_vbox = GTK_BOX(gtk_vbox_new(false, 8));
	gtk_container_add(GTK_CONTAINER(page), GTK_WIDGET(appearance_vbox));

	// Add option to use generic names
	m_show_names = gtk_check_button_new_with_mnemonic(_("Show applications by _name"));
	gtk_box_pack_start(appearance_vbox, m_show_names, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_names), Launcher::get_show_name());
	g_signal_connect(m_show_names, "toggled", G_CALLBACK(ConfigurationDialog::toggle_show_name_slot), this);

	// Add option to hide descriptions
	m_show_descriptions = gtk_check_button_new_with_mnemonic(_("Show application _descriptions"));
	gtk_box_pack_start(appearance_vbox, m_show_descriptions, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_show_descriptions), Launcher::get_show_description());
	g_signal_connect(m_show_descriptions, "toggled", G_CALLBACK(ConfigurationDialog::toggle_show_description_slot), this);

	// Add item icon size selector
	GtkBox* hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(appearance_vbox, GTK_WIDGET(hbox), false, false, 0);

	GtkWidget* label = gtk_label_new_with_mnemonic(_("Ite_m icon size:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_item_icon_size = gtk_combo_box_text_new();
	std::vector<std::string> icon_sizes = IconSize::get_strings();
	for (std::vector<std::string>::const_iterator i = icon_sizes.begin(), end = icon_sizes.end(); i != end; ++i)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_item_icon_size), i->c_str());
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_item_icon_size), LauncherView::get_icon_size());
	gtk_box_pack_start(hbox, m_item_icon_size, false, false, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_item_icon_size);
	g_signal_connect(m_item_icon_size, "changed", G_CALLBACK(ConfigurationDialog::item_icon_size_changed_slot), this);

	// Add category icon size selector
	hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(appearance_vbox, GTK_WIDGET(hbox), false, false, 0);

	label = gtk_label_new_with_mnemonic(_("Categ_ory icon size:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_category_icon_size = gtk_combo_box_text_new();
	for (std::vector<std::string>::const_iterator i = icon_sizes.begin(), end = icon_sizes.end(); i != end; ++i)
	{
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_category_icon_size), i->c_str());
	}
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_category_icon_size), SectionButton::get_icon_size());
	gtk_box_pack_start(hbox, m_category_icon_size, false, false, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_category_icon_size);
	g_signal_connect(m_category_icon_size, "changed", G_CALLBACK(ConfigurationDialog::category_icon_size_changed_slot), this);

	return page;
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_panel_button_tab()
{
	// Create size group for labels
	GtkSizeGroup* label_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	// Create panel button section
	GtkWidget* page = gtk_alignment_new(0, 0, 1, 0);
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);
	GtkBox* panel_vbox = GTK_BOX(gtk_vbox_new(false, 8));
	gtk_container_add(GTK_CONTAINER(page), GTK_WIDGET(panel_vbox));

	// Add button style selector
	GtkBox* hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(panel_vbox, GTK_WIDGET(hbox), false, false, 0);

	GtkWidget* label = gtk_label_new_with_mnemonic(_("Di_splay:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_button_style = gtk_combo_box_text_new();
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Icon"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Title"));
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(m_button_style), _("Icon and title"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(m_button_style), static_cast<int>(m_plugin->get_button_style()) - 1);
	gtk_box_pack_start(hbox, m_button_style, false, false, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_button_style);
	g_signal_connect(m_button_style, "changed", G_CALLBACK(ConfigurationDialog::style_changed_slot), this);

	// Add title selector
	hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(panel_vbox, GTK_WIDGET(hbox), false, false, 0);

	label = gtk_label_new_with_mnemonic(_("_Title:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_title = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_title), m_plugin->get_button_title().c_str());
	gtk_box_pack_start(hbox, m_title, true, true, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_title);
	g_signal_connect(m_title, "changed", G_CALLBACK(ConfigurationDialog::title_changed_slot), this);

	// Add icon selector
	hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(panel_vbox, GTK_WIDGET(hbox), false, false, 0);

	label = gtk_label_new_with_mnemonic(_("_Icon:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_icon_button = gtk_button_new();
	gtk_box_pack_start(hbox, m_icon_button, false, false, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_icon_button);
	g_signal_connect(m_icon_button, "clicked", G_CALLBACK(ConfigurationDialog::choose_icon_slot), this);

	m_icon = xfce_panel_image_new_from_source(m_plugin->get_button_icon_name().c_str());
	xfce_panel_image_set_size(XFCE_PANEL_IMAGE(m_icon), 48);
	gtk_container_add(GTK_CONTAINER(m_icon_button), m_icon);

	return page;
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_behavior_tab()
{
	// Create behavior section
	GtkWidget* page = gtk_alignment_new(0, 0, 1, 0);
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);
	GtkBox* behavior_vbox = GTK_BOX(gtk_vbox_new(false, 8));
	gtk_container_add(GTK_CONTAINER(page), GTK_WIDGET(behavior_vbox));

	// Add option to use generic names
	m_hover_switch_category = gtk_check_button_new_with_mnemonic(_("Switch categories by _hovering"));
	gtk_box_pack_start(behavior_vbox, m_hover_switch_category, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_hover_switch_category), SectionButton::get_hover_activate());
	g_signal_connect(m_hover_switch_category, "toggled", G_CALLBACK(ConfigurationDialog::toggle_hover_switch_category_slot), this);

	// Add option to load menu hierarchy
	m_load_hierarchy = gtk_check_button_new_with_mnemonic(_("Load menu hie_rarchy"));
	gtk_box_pack_start(behavior_vbox, m_load_hierarchy, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_load_hierarchy), ApplicationsPage::get_load_hierarchy());
	g_signal_connect(m_load_hierarchy, "toggled", G_CALLBACK(ConfigurationDialog::toggle_load_hierarchy_slot), this);

	// Add option to remember favorites
	m_remember_favorites = gtk_check_button_new_with_mnemonic(_("Include _favorites in recently used"));
	gtk_box_pack_start(behavior_vbox, m_remember_favorites, true, true, 0);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(m_remember_favorites), FavoritesPage::get_remember_favorites());
	g_signal_connect(m_remember_favorites, "toggled", G_CALLBACK(ConfigurationDialog::toggle_remember_favorites_slot), this);

	return page;
}

//-----------------------------------------------------------------------------

GtkWidget* ConfigurationDialog::init_commands_tab()
{
	// Create size group for labels
	GtkSizeGroup* label_size_group = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	// Create commands section
	GtkWidget* page = gtk_alignment_new(0, 0, 1, 0);
	gtk_container_set_border_width(GTK_CONTAINER(page), 8);
	GtkBox* panel_vbox = GTK_BOX(gtk_vbox_new(false, 8));
	gtk_container_add(GTK_CONTAINER(page), GTK_WIDGET(panel_vbox));

	// Add settings command entry
	GtkBox* hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(panel_vbox, GTK_WIDGET(hbox), false, false, 0);

	GtkWidget* label = gtk_label_new_with_mnemonic(_("_Settings:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_settings_command = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_settings_command), Menu::get_settings_command().c_str());
	gtk_box_pack_start(hbox, m_settings_command, true, true, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_settings_command);
	g_signal_connect(m_settings_command, "changed", G_CALLBACK(ConfigurationDialog::settings_command_changed_slot), this);

	// Add lock screen command entry
	hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(panel_vbox, GTK_WIDGET(hbox), false, false, 0);

	label = gtk_label_new_with_mnemonic(_("Lock _Screen:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_lockscreen_command = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_lockscreen_command), Menu::get_lockscreen_command().c_str());
	gtk_box_pack_start(hbox, m_lockscreen_command, true, true, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_lockscreen_command);
	g_signal_connect(m_lockscreen_command, "changed", G_CALLBACK(ConfigurationDialog::lockscreen_command_changed_slot), this);

	// Add log out command entry
	hbox = GTK_BOX(gtk_hbox_new(false, 12));
	gtk_box_pack_start(panel_vbox, GTK_WIDGET(hbox), false, false, 0);

	label = gtk_label_new_with_mnemonic(_("Log _Out:"));
	gtk_misc_set_alignment(GTK_MISC(label), 0, 0.5);
	gtk_box_pack_start(hbox, label, false, false, 0);
	gtk_size_group_add_widget(label_size_group, label);

	m_logout_command = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(m_logout_command), Menu::get_logout_command().c_str());
	gtk_box_pack_start(hbox, m_logout_command, true, true, 0);
	gtk_label_set_mnemonic_widget(GTK_LABEL(label), m_logout_command);
	g_signal_connect(m_logout_command, "changed", G_CALLBACK(ConfigurationDialog::logout_command_changed_slot), this);

	return page;
}

//-----------------------------------------------------------------------------
