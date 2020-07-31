// Fill out your copyright notice in the Description page of Project Settings.

#include "L2DCubismEd.h"
#include "Modules/ModuleManager.h"
//#include "Textures/SlateIcon.h"
// #include "L2DCubismEditMode.h"

#define LOCTEXT_NAMESPACE "FL2DCubismModuleEd"

void IL2DCubismEdModule::StartupModule()
{
}

void IL2DCubismEdModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(IL2DCubismEdModule, L2DCubismEd)
//IMPLEMENT_PRIMARY_GAME_MODULE( FDefaultGameModuleImpl, L2DCubismEd, "L2DCubismEd" );
