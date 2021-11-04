// Created by wangpeimin@corp.netease.com

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DShowCameraComponent.generated.h"


USTRUCT(BlueprintType)
struct FCameraDataBuffer
{
	GENERATED_USTRUCT_BODY()

	FCameraDataBuffer()
	{
		FCameraDataBuffer(nullptr, 0, 0);
	}
	FCameraDataBuffer(uint8* Buffer, int32 Width, int32 Height)
	{
		DataBuffer = Buffer;
		DataWidth = Width;
		DataHeight = Height;
	}
	uint8* DataBuffer;
	int32 DataWidth;
	int32 DataHeight;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBufferCallBack, const FCameraDataBuffer&, CameraDataBuffer);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DSHOWCAMERA_API UDShowCameraComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UDShowCameraComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason)override;
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	UFUNCTION(BlueprintCallable, Category = "DShowCameraComponent")
	void GetDeviceList(TArray<FString>& DeviceNames);

	UFUNCTION(BlueprintCallable, Category = "DShowCameraComponent")
	void OpenDevice(const FString& DeviceName, int32 Width, int32 Height, int32 FPS);

	UFUNCTION()
	void OnReceivedDataBuffer(const FCameraDataBuffer& CameraDataBuffer);

	UPROPERTY(BlueprintAssignable)
	FBufferCallBack BufferCallBack;


	UPROPERTY(EditAnywhere)
	bool bShowCameraPicture;

	UPROPERTY(BlueprintReadOnly)
	UTexture2D* CameraPicture;
private:
	class DSCamera* Camera;
	FCameraDataBuffer CameraDataBuffer;
		
	void UpdateCameraTexture(const FCameraDataBuffer& CameraDataBuffer);
};
