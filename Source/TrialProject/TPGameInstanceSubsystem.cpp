// Fill out your copyright notice in the Description page of Project Settings.


#include "TPGameInstanceSubsystem.h"
#include "OnlineSubsystemUtils.h"


UTPGameInstanceSubsystem::UTPGameInstanceSubsystem()
	: CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UTPGameInstanceSubsystem::OnCreateSessionComplete))
	, StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &UTPGameInstanceSubsystem::OnStartSessionComplete))
	, EndSessionCompleteDelegate(FOnEndSessionCompleteDelegate::CreateUObject(this, &UTPGameInstanceSubsystem::OnEndSessionComplete))
	, DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UTPGameInstanceSubsystem::OnDestroySessionComplete))
	, FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UTPGameInstanceSubsystem::OnFindSessionsComplete))
	, JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UTPGameInstanceSubsystem::OnJoinSessionComplete))
{
}

// Create Session
void UTPGameInstanceSubsystem::CreateSession(int32 NumPublicConnections, bool IsLANMatch)
{
	if (const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		// Session Settings
		SessionSettings = MakeShareable(new FOnlineSessionSettings());
		SessionSettings->NumPrivateConnections = 0;
		SessionSettings->NumPublicConnections = NumPublicConnections;
		SessionSettings->bAllowJoinInProgress = true;
		SessionSettings->bIsDedicated = false;
		SessionSettings->bIsLANMatch = IsLANMatch;
		SessionSettings->bUsesPresence = true;
		SessionSettings->bAllowJoinViaPresence = true;
		SessionSettings->bShouldAdvertise = true;
		SessionSettings->Set(SETTING_MAPNAME, FString("ThirdPersonMap"), EOnlineDataAdvertisementType::ViaOnlineService);

		CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);
		if (const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController())
		{
			if (!SessionInterface->CreateSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
			{
				SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
				OnCreateSessionCompleteEvent.Broadcast(false);
			}
		}
	}
	else
	{
		OnCreateSessionCompleteEvent.Broadcast(false);
		return;
	}
}

void UTPGameInstanceSubsystem::OnCreateSessionComplete(FName SessionName, bool Successful)
{
	if (const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	}
	OnCreateSessionCompleteEvent.Broadcast(Successful);
	GetWorld()->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
}

// Start Session
void UTPGameInstanceSubsystem::StartSession()
{
	if (const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);
		if (!SessionInterface->StartSession(NAME_GameSession))
		{
			SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
			OnStartSessionCompleteEvent.Broadcast(false);
		}
	}
	else
	{
		OnStartSessionCompleteEvent.Broadcast(false);
		return;
	}
}

void UTPGameInstanceSubsystem::OnStartSessionComplete(FName SessionName, bool Successful)
{
	if (const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
	}
	OnStartSessionCompleteEvent.Broadcast(Successful);
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Green, TEXT("Started Game Session"));
}

// End Session
void UTPGameInstanceSubsystem::EndSession()
{
	if (const IOnlineSessionPtr SessionInterface = Online::GetSessionInterface(GetWorld()))
	{
		EndSessionCompleteDelegateHandle = SessionInterface->AddOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegate);
		if (!SessionInterface->EndSession(NAME_GameSession))
		{
			SessionInterface->ClearOnEndSessionCompleteDelegate_Handle(EndSessionCompleteDelegateHandle);
			OnEndSessionCompleteEvent.Broadcast(false);
		}
	}
	else
	{
		OnEndSessionCompleteEvent.Broadcast(false);
		return;
	}
}

void UTPGameInstanceSubsystem::OnEndSessionComplete(FName SessionName, bool Successful)
{

}

// Destroy Session
void UTPGameInstanceSubsystem::DestroySession()
{

}

void UTPGameInstanceSubsystem::OnDestroySessionComplete(FName SessionName, bool Successful)
{

}

void UTPGameInstanceSubsystem::FindSessions(int32 MaxSearchResults, bool IsLANQuery)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	FindSessionsCompleteDelegateHandle =
		sessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	SessionSearch->MaxSearchResults = MaxSearchResults;
	SessionSearch->bIsLanQuery = IsLANQuery;

	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->FindSessions(*localPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef()))
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
	}
}

void UTPGameInstanceSubsystem::OnFindSessionsComplete(bool Successful)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	}

	if (SessionSearch->SearchResults.Num() <= 0)
	{
		OnFindSessionsCompleteEvent.Broadcast(TArray<FOnlineSessionSearchResult>(), Successful);
		return;
	}
	OnFindSessionsCompleteEvent.Broadcast(SessionSearch->SearchResults, Successful);

	if (GEngine && SessionSearch->SearchResults.Num() > 0)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Green, TEXT("found session, joining"));
		JoinGameSession(SessionSearch->SearchResults[0]); // join the first session found (for testing locally)
	}
}

void UTPGameInstanceSubsystem::JoinGameSession(const FOnlineSessionSearchResult& SessionResult)
{

	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		OnJoinGameSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
		return;
	}

	JoinSessionCompleteDelegateHandle = sessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	const ULocalPlayer* localPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!sessionInterface->JoinSession(*localPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionResult))
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		OnJoinGameSessionCompleteEvent.Broadcast(EOnJoinSessionCompleteResult::UnknownError);
	}
}

void UTPGameInstanceSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (sessionInterface)
	{
		sessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}
	OnJoinGameSessionCompleteEvent.Broadcast(Result);
	TryTravelToCurrentSession(); // travel to the session
}

bool UTPGameInstanceSubsystem::TryTravelToCurrentSession()
{
	const IOnlineSessionPtr sessionInterface = Online::GetSessionInterface(GetWorld());
	if (!sessionInterface.IsValid())
	{
		return false;
	}
	FString connectString;
	if (!sessionInterface->GetResolvedConnectString(NAME_GameSession, connectString))
	{
		return false;
	}
	APlayerController* playerController = GetWorld()->GetFirstPlayerController();
	playerController->ClientTravel(connectString, TRAVEL_Absolute);
	return true;
}

// for testing
void UTPGameInstanceSubsystem::UCreateSession()
{
	CreateSession(10, true);
}

void UTPGameInstanceSubsystem::UStartSession()
{
	StartSession();
}

void UTPGameInstanceSubsystem::UFindSessions()
{
	FindSessions(16, true);
}