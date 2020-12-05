// Fill out your copyright notice in the Description page of Project Settings.


#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	InventorySubsystem = GetWorld()->GetGameInstance()->GetSubsystem<UInventorySubsystem>();
}

void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}




TArray<FItemSlotInfo> UInventoryComponent::GetContent()
{
	TArray<FItemSlotInfo> Content;
	for (FItemSlot Slot : InnerContent)
	{
		FItemSlotInfo SlotInfo;

		if (UItemBase* FoundedItem = FindObject<UItemBase>(GetWorld()->GetGameInstance(), *FString::Printf(TEXT("Item_%llu"), Slot.ItemID), false))
		{
			SlotInfo.Item = FoundedItem;
			SlotInfo.Count = Slot.Count;
			Content.Add(SlotInfo);
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 15, FColor::Red, FString::Printf(TEXT("Item_%llu NOT FOUND"), Slot.ItemID));
		}
	}
	return Content;
}

void UInventoryComponent::AddItems(UItemBase* Item, int32 Amount)
{
	if(FItemSlot* Slot = InnerContent.FindByPredicate([&](FItemSlot Result) {return Item->ItemID == Result.ItemID; }))
	{
		Slot->Count += Amount;
	} else
	{
		FItemSlot NewSlot;
		NewSlot.ItemID = Item->ItemID;
		NewSlot.Count = Amount;
		InnerContent.Add(NewSlot);
	}

	InventorySubsystem->IncreaseItemCount(Item, Amount);
	return;
}

void UInventoryComponent::RemoveItems(UItemBase* Item, int32 Amount)
{
	if(FItemSlot* Slot = InnerContent.FindByPredicate([&](FItemSlot Result) { return Item->ItemID == Result.ItemID; }))
	{
		if(Slot->Count>Amount)
		{
			InventorySubsystem->IncreaseItemCount(Item, Amount);
			Slot->Count -= Amount;
		}
		else
		{
			InventorySubsystem->IncreaseItemCount(Item, Slot->Count);
			InnerContent.Remove(*Slot);
		}
	}
	return;
}

bool UInventoryComponent::TransactTo(UItemBase* Item, int32 Amount, UInventoryComponent* To)
{
	if(IsContain(Item, Amount) && To->CanAccept(Item, Amount))
	{
		RemoveItems(Item, Amount);
		To->AddItems(Item, Amount);
	}
	return true;
}

bool UInventoryComponent::IsContain(UItemBase* Item, int32 Amount)
{
	if(FItemSlot* Slot = InnerContent.FindByPredicate([&](FItemSlot Result) {return Item->ItemID == Result.ItemID; }))
	{
		return Slot->Count>=Amount;
	}
	return false;
}

bool UInventoryComponent::CanAccept_Implementation(UItemBase* Item, int32 Amount)
{
	return true;
}
