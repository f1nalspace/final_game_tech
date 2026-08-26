/*
Name:
	Final Gamebox

	Frontend-Part (About Dialog Header)

Author:
	Torsten Spaete

Description:
	This file is part of the frontend of the Final Gamebox project.

	Everything a new user has no other way of finding out: which keys and which gamepad buttons the
	joypad is wired to, what the application can actually do, who wrote it and what it was built out of.

	The pages are plain text laid out in columns and shown in a text view that does NOT wrap, because
	a column that wraps is a column no more.
*/

#pragma once

#include <final_ui.h>

#include "ui.h"

// Which page of the dialog is showing
typedef enum {
	AboutPage_About = 0,
	AboutPage_HowToUse,
	AboutPage_Controls,
	AboutPage_Features,
	AboutPage_Libraries,
	AboutPage_Count,
} AboutPage;

typedef struct {
	// One scroll position per page, so switching back to a page returns to where it was left
	UITextViewState pageViews[AboutPage_Count];
	int32_t selectedPageIndex;
} AboutDialog;

// Identifies the dialog to the library, and is what AboutDialogOpen and its build agree on
extern const char *AboutDialogId;

// Opens the dialog showing a particular page, which is what lets more than one button lead into it
extern void AboutDialogOpen(fuiContext *ui, AboutDialog *dialog, const AboutPage page);

// Builds the dialog when it is open and does nothing at all when it is not, so this may be called every frame.
// titleIcon is worn in the title bar, so a small bake of it is what belongs here. backdropIcon is the
// application's own icon, laid faintly across the whole dialog behind its pages.
extern void AboutDialogBuild(fuiContext *ui, AboutDialog *dialog, const Texture *titleIcon, const Texture *backdropIcon, const float windowWidth, const float windowHeight);
