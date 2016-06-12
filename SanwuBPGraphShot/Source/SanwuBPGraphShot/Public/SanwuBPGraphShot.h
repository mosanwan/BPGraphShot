// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ModuleManager.h"
#include "GraphEditor.h"
//#include "TabManager.h"
class FToolBarBuilder;
class FMenuBuilder;

struct FShotImageData
{
public:
	TArray<TArray<FColor>>SerializedData;
	TArray<FColor> ImageData;
	FIntVector ImageSize;
	int32 LocationX;
	int32 LocationY;
	FIntRect InnerShotArea;
};
class FSanwuBPGraphShotModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
	/** This function will be bound to Command. */
	void PluginButtonClicked();
	
private:

	void AddToolbarExtension(FToolBarBuilder& Builder);
	void AddMenuExtension(FMenuBuilder& Builder);
	void OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow);

	void HandleGraphFind(SGraphEditor* graph,TSharedRef<SWidget>Content);
	void ClipboardCopy(TArray<FColor> BitmapData, FIntVector Size);
	void UpdateShotTimer();
	void FinishedShot();
private:
	void GetChildrenRecursion(FChildren*childrens);
	
	TSharedPtr<SWidget>GraphEditorWidget;
	TSharedPtr<class FUICommandList> PluginCommands;

	FTimerHandle ShotTimerHandler;
	FTimerDelegate OnTimerDelegate;
	TArray<FShotImageData> MissonPool;
	int32 CurrentMissionIndex = 0;

	SGraphEditor* CurrentGraphEditor;
	TSharedPtr<SWidget> CurrentContent;



	FVector2D ContentSize;
	int32 MaxNodeX;
	int32 MaxNodeY;
	int32 MinNodeX;
	int32 MinNodeY;

	int32 SpaceWidth;
	int32 SpaceHeight;

	int32 TotalWidth;
	int32 TotalHeight;
};