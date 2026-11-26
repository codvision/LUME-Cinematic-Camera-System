#include "LumeCameraZoneProfile.h"

void ULumeCameraZoneProfile::ApplyPreset()
{
	if (Preset != ELumeZonePreset::Custom)
	{
		Modify();
		Framing = FLumeFramingOverrides::MakePreset(Preset);
	}
}

#if WITH_EDITOR
void ULumeCameraZoneProfile::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(ULumeCameraZoneProfile, Preset))
	{
		ApplyPreset();
	}
}
#endif
