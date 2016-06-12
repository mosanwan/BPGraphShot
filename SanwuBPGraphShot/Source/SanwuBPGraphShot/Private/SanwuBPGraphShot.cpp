// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "SanwuBPGraphShotPrivatePCH.h"

#include "SlateBasics.h"
#include "SlateExtras.h"
#include "Runtime/Slate/Private/Framework/Docking/DockingPrivate.h"

#include "TabManager.h"
#include "SanwuBPGraphShotStyle.h"
#include "SanwuBPGraphShotCommands.h"

#include "IMainFrameModule.h"
#include "BlueprintEditorModule.h"
#include "GraphEditor.h"
#include "LevelEditor.h"
#include "Kismet/KismetMathLibrary.h"

static const FName SanwuBPGraphShotTabName("SanwuBPGraphShot");

#define LOCTEXT_NAMESPACE "FSanwuBPGraphShotModule"
DEFINE_LOG_CATEGORY_STATIC(LogBPShot, Warning, All);

void FSanwuBPGraphShotModule::StartupModule()
{
	FSanwuBPGraphShotStyle::Initialize();
	FSanwuBPGraphShotStyle::ReloadTextures();

	FSanwuBPGraphShotCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FSanwuBPGraphShotCommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSanwuBPGraphShotModule::PluginButtonClicked),
		FCanExecuteAction());
		
	//
	IMainFrameModule& MainFrameModule = FModuleManager::LoadModuleChecked<IMainFrameModule>(TEXT("MainFrame"));
	MainFrameModule.OnMainFrameCreationFinished().AddRaw(this, &FSanwuBPGraphShotModule::OnMainFrameLoad);

	OnTimerDelegate.BindRaw(this, &FSanwuBPGraphShotModule::UpdateShotTimer);
}
void FSanwuBPGraphShotModule::OnMainFrameLoad(TSharedPtr<SWindow> InRootWindow, bool bIsNewProjectWindow)
{
	FBlueprintEditorModule& BlueprintEditorModule = FModuleManager::LoadModuleChecked<FBlueprintEditorModule>("Kismet");
	{
		TSharedPtr<FExtender> BPMenuExtender = MakeShareable(new FExtender());
		BPMenuExtender->AddToolBarExtension("Asset", EExtensionHook::After, PluginCommands, FToolBarExtensionDelegate::CreateRaw(this, &FSanwuBPGraphShotModule::AddToolbarExtension));
		BlueprintEditorModule.GetMenuExtensibilityManager()->AddExtender(BPMenuExtender);
	}
}
void FSanwuBPGraphShotModule::ShutdownModule()
{
	FSanwuBPGraphShotStyle::Shutdown();
	FSanwuBPGraphShotCommands::Unregister();
}

void FSanwuBPGraphShotModule::PluginButtonClicked()
{
	TSharedPtr<SDockTab>Tab=FGlobalTabmanager::Get()->GetActiveTab();
	TSharedPtr<class SDockingArea>TabDockingArea= Tab->GetDockArea();
	TArray<TSharedRef<SDockTab>>AllTab= TabDockingArea->GetAllChildTabs();

	TSharedPtr<SWindow>TabParentWin = FSlateApplication::Get().FindWidgetWindow(Tab.ToSharedRef());

	for (TSharedRef<SDockTab>TabIt : AllTab)
	{
		TSharedRef<SWidget>TabContent = TabIt->GetContent();
		if (TabIt->IsForeground()&& TabContent->ToString().Contains("SGraphEditor"))
		{
			//UE_LOG(LogBPShot, Warning, TEXT("%s  -> %s "), *(TabIt->GetTabLabel().ToString()), *(TabContent->ToString()));
			SGraphEditor* CurrentGraphEditor = (SGraphEditor*)&TabContent.Get();
			FVector2D ViewLocation;
			float ZoomScale = 1.0f;
			CurrentGraphEditor->GetViewLocation(ViewLocation, ZoomScale);
			UE_LOG(LogBPShot, Warning, TEXT("%f  -> %f "), ViewLocation.X,ViewLocation.Y);
			if (CurrentGraphEditor)
			{
				GetChildrenRecursion(CurrentGraphEditor->GetChildren());
				HandleGraphFind(CurrentGraphEditor,TabContent);
			}
		}
	}
}
void FSanwuBPGraphShotModule::GetChildrenRecursion(FChildren*childrens)
{
	if (childrens->Num() > 0)
	{
		for (int32 i = 0; i < childrens->Num(); i++)
		{
			TSharedRef<SWidget> Child = childrens->GetChildAt(i);
			FString ChildName = Child->GetTypeAsString();

			if (ChildName == FString("SGraphPanel")) //SLevelEditorWatermark  
			{
				//c->SetVisibility(EVisibility::Collapsed);
				//GraphPanelWidget = Child;
			}
			if (ChildName == FString("STextBlock")) //
			{
				Child->SetVisibility(EVisibility::Collapsed);
			}
			if (ChildName == FString("SGraphTitleBar")) //
			{
				Child->SetVisibility(EVisibility::Collapsed);
			}
			if (!ChildName.Contains("SGraphNode"))
			{
				GetChildrenRecursion(Child->GetChildren());
			}
			
		}
	}
}
void FSanwuBPGraphShotModule::HandleGraphFind(SGraphEditor* graph,TSharedRef<SWidget>Content)
{
	CurrentGraphEditor = graph;
	CurrentContent = Content;

	TArray<FColor> OutData;
	FIntVector OutSize;
	FSlateApplication::Get().TakeScreenshot(Content, OutData, OutSize);
// 	ClipboardCopy(OutData, OutSize);
// 	UE_LOG(LogBPShot, Warning, TEXT("Snapshot width =%d height= %d"), OutSize.X, OutSize.Y);
	UEdGraph*GraphObj = graph->GetCurrentGraph();

	
	TArray<int32> PosXs;
	TArray<int32> PosYs;
	for (class UEdGraphNode* Node : GraphObj->Nodes)
	{
		int32 currentX= Node->NodePosX;
		int32 currentY= Node->NodePosY;
		PosXs.Add(currentX);
		PosYs.Add(currentY);
		int32 currentWidth = Node->NodeWidth;
		int32 currentHeight = Node->NodeHeight;
	}

	PosXs.Sort();
	PosYs.Sort();

	int32 PaddingSpace = 50;
	//Get Sizes
	ContentSize.X = OutSize.X;
	ContentSize.Y = OutSize.Y;

	 MaxNodeX = PosXs[PosXs.Num() - 1]+ PaddingSpace;
	 MaxNodeY = PosYs[PosYs.Num() - 1]+ PaddingSpace;
	 MinNodeX = PosXs[0]- PaddingSpace;
	 MinNodeY = PosYs[0]- PaddingSpace;

	 SpaceWidth = MaxNodeX - MinNodeX;
	 SpaceHeight = MaxNodeY - MinNodeY;

	float Remainder = 0;

	int32 ColCount = UKismetMathLibrary::FMod(SpaceWidth, ContentSize.X, Remainder);
	ColCount += Remainder > 0 ? 1 : 0;

	int32 RowCount = UKismetMathLibrary::FMod(SpaceHeight, ContentSize.Y, Remainder);
	RowCount += Remainder > 0 ? 1 : 0;


	TotalWidth = ContentSize.X*ColCount;
	TotalHeight = ContentSize.Y*RowCount;

	//
	//

	//Save Image
	FVector2D OriginViewLocation;
	float OriginZoom;
	graph->GetViewLocation(OriginViewLocation, OriginZoom);
	

	MissonPool.Empty();

	for (int32 i =0;i<RowCount;i++)
	{
		for (int32 j=0;j<ColCount;j++)
		{
			TArray<FColor> ImageData;
			int32 px = j*ContentSize.X + MinNodeX;
			int32 py = i*ContentSize.Y + MinNodeY;
			
			FIntRect ShotRect = FIntRect(0, 0,ContentSize.X , ContentSize.Y);
			if (px+ContentSize.X >MaxNodeX )
			{
				//ShotRect.Max.X = MaxNodeX - px;
			}
			if (py+ContentSize.Y>MaxNodeY)
			{
				//ShotRect.Max.Y = MaxNodeY - py;
			}
			FShotImageData Mission = FShotImageData();
			Mission.InnerShotArea = ShotRect;
			Mission.LocationX = px;
			Mission.LocationY = py;
			MissonPool.Add(Mission);
		}
		CurrentMissionIndex = 0;
		UpdateTrigger = false;
		GUnrealEd->GetTimerManager()->SetTimer(ShotTimerHandler, OnTimerDelegate, 0.5, true);
	}

// 	graph->SetViewLocation(FVector2D(px, py), 1);
// 	UE_LOG(LogBPShot, Warning, TEXT(" %d  %d"), px, py);
// 	FSlateApplication::Get().TakeScreenshot(Content,/*ShotRect, */ImageData, OutSize);
// 	FFileHelper::CreateBitmap(*(FPaths::GameSavedDir() / "BlueprintGraphShot/"), OutSize.X, OutSize.Y, OutData.GetData());
	//graph->SetViewLocation(OriginViewLocation, OriginZoom);
	
	//
	//UE_LOG(LogBPShot, Warning, TEXT(" %d  %d"), ColCount,RowCount);
}
void FSanwuBPGraphShotModule::UpdateShotTimer()
{
	UpdateTrigger = !UpdateTrigger;

	FShotImageData& Mission= MissonPool[CurrentMissionIndex];
	if (UpdateTrigger)
	{
		CurrentGraphEditor->SetViewLocation(FVector2D(Mission.LocationX, Mission.LocationY), 1);
	}
	else {
		FSlateApplication::Get().TakeScreenshot(CurrentContent->AsShared(), Mission.InnerShotArea, Mission.ImageData, Mission.ImageSize);

		CurrentMissionIndex++;

		if (CurrentMissionIndex > MissonPool.Num() - 1)
		{
			GUnrealEd->GetTimerManager()->ClearTimer(ShotTimerHandler);
			FinishedShot();
		}
	}
}
void FSanwuBPGraphShotModule::FinishedShot()
{
	for (FShotImageData & ImageData : MissonPool)
	{
		//FFileHelper::CreateBitmap(*(FPaths::GameSavedDir() / "BlueprintGraphShot/"), ImageData.ImageSize.X, ImageData.ImageSize.Y, ImageData.ImageData.GetData());

		for (int32 i=0;i<ImageData.ImageSize.Y;i++)
		{
			TArray<FColor> rowData;
			for (int32 j = 0; j < ImageData.ImageSize.X; j++)
			{
				rowData.Add(ImageData.ImageData[ImageData.ImageSize.X*i + j]);
			}
			ImageData.SerializedData.Add(rowData);
		}
	}
	//return;

	//
	

	TArray<TArray<FColor>> PreFinishData;
	FShotImageData& PreImageData = MissonPool[0];

	for (int32 h=0;h<TotalHeight;h++)
	{
		PreFinishData.Add(TArray<FColor>());
	}
	
	for (FShotImageData & ImageData : MissonPool)
	{
		for ( int32 p=0;p<ImageData.SerializedData.Num();p++ )
		{
			TArray<FColor> rArray = ImageData.SerializedData[p];
			int32 fIndex = ImageData.LocationY - MinNodeY + p;
			TArray<FColor>& PreArray = PreFinishData[fIndex];
			PreArray.Append(rArray);
		}
	}
	

	TArray<FColor> FinalData;
	for (TArray<FColor>pice : PreFinishData)
	{
		FinalData.Append(pice);
	}

	FFileHelper::CreateBitmap(*(FPaths::GameSavedDir() / "BlueprintGraphShot/"),TotalWidth,TotalHeight, FinalData.GetData());

}

void FSanwuBPGraphShotModule::AddToolbarExtension(FToolBarBuilder& Builder)
{
	Builder.AddToolBarButton(FSanwuBPGraphShotCommands::Get().PluginAction);
}

void FSanwuBPGraphShotModule::ClipboardCopy(TArray<FColor> BitmapData, FIntVector Size)
{
	
	//HBITMAP


	const TCHAR* Str = TEXT("Hello World");
	if (OpenClipboard(GetActiveWindow()))
	{
		verify(EmptyClipboard());
		HGLOBAL GlobalMem;
		int32 StrLen = FCString::Strlen(Str);
		GlobalMem = GlobalAlloc(GMEM_MOVEABLE, sizeof(TCHAR)*(StrLen + 1));
		check(GlobalMem);
		TCHAR* Data = (TCHAR*)GlobalLock(GlobalMem);
		FCString::Strcpy(Data, (StrLen + 1), Str);
		GlobalUnlock(GlobalMem);
		if (SetClipboardData(CF_BITMAP, GlobalMem) == NULL)
			UE_LOG(LogWindows, Fatal, TEXT("SetClipboardData failed with error code %i"), (uint32)GetLastError());
		verify(CloseClipboard());
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSanwuBPGraphShotModule, SanwuBPGraphShot)