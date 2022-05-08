// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "TPGameInstanceSubsystem.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTPOnCreateSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTPOnStartSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTPOnEndSessionComplete, bool, Successful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTPOnDestroySessionComplete, bool, Successful);
DECLARE_MULTICAST_DELEGATE_OneParam(FTPOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_MULTICAST_DELEGATE_TwoParams(FTPOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool Successful);

UCLASS()
class TRIALPROJECT_API UTPGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UTPGameInstanceSubsystem();

	void CreateSession(int32 NumPublicConnections, bool IsLANMatch);
	void StartSession();
	void EndSession();
	void DestroySession();
	void FindSessions(int32 MaxSearchResults, bool IsLANQuery);
	void JoinGameSession(const FOnlineSessionSearchResult& SessionResult);

	FTPOnCreateSessionComplete OnCreateSessionCompleteEvent;
	FTPOnStartSessionComplete OnStartSessionCompleteEvent;
	FTPOnEndSessionComplete OnEndSessionCompleteEvent;
	FTPOnDestroySessionComplete OnDestroySessionCompleteEvent;
	FTPOnFindSessionsComplete OnFindSessionsCompleteEvent;
	FTPOnJoinSessionComplete OnJoinGameSessionCompleteEvent;

protected:
	void OnCreateSessionComplete(FName SessionName, bool Successful);
	void OnStartSessionComplete(FName SessionName, bool Successful);
	void OnEndSessionComplete(FName SessionName, bool Successful);
	void OnDestroySessionComplete(FName SessionName, bool Successful);
	void OnFindSessionsComplete(bool Successful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	bool TryTravelToCurrentSession();

private:
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnEndSessionCompleteDelegate EndSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FDelegateHandle EndSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	// for testing
public:
	UFUNCTION(BlueprintCallable)
	void UCreateSession();

	UFUNCTION(BlueprintCallable)
	void UFindSessions();

	UFUNCTION(BlueprintCallable)
	void UStartSession();
};
