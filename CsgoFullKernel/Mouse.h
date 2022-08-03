#pragma once
#include "Misc.h"



extern "C" POBJECT_TYPE * IoDriverObjectType;

typedef VOID
(*MouseClassServiceCallback)(
	PDEVICE_OBJECT DeviceObject,
	PMOUSE_INPUT_DATA InputDataStart,
	PMOUSE_INPUT_DATA InputDataEnd,
	PULONG InputDataConsumed
	);

typedef struct _MOUSE_OBJECT
{
	PDEVICE_OBJECT mouse_device;
	MouseClassServiceCallback service_callback;
} MOUSE_OBJECT, * PMOUSE_OBJECT;

MOUSE_OBJECT mouseObject;

NTSTATUS InitMouse(PMOUSE_OBJECT mouse_obj)
{
	UNICODE_STRING class_string;
	RtlInitUnicodeString(&class_string, L"\\Driver\\MouClass");

	PDRIVER_OBJECT class_driver_object = NULL;
	NTSTATUS status = ObReferenceObjectByName(&class_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&class_driver_object);
	if (!NT_SUCCESS(status)) { return status; }

	UNICODE_STRING hid_string;
	RtlInitUnicodeString(&hid_string, L"\\Driver\\MouHID");

	PDRIVER_OBJECT hid_driver_object = NULL;
	status = ObReferenceObjectByName(&hid_string, OBJ_CASE_INSENSITIVE, NULL, 0, *IoDriverObjectType, KernelMode, NULL, (PVOID*)&hid_driver_object);
	if (!NT_SUCCESS(status))
	{
		if (class_driver_object) { ObDereferenceObject(class_driver_object); }
		return status;
	}

	PVOID class_driver_base = NULL;

	PDEVICE_OBJECT hid_device_object = hid_driver_object->DeviceObject;
	while (hid_device_object && !mouse_obj->service_callback)
	{
		PDEVICE_OBJECT class_device_object = class_driver_object->DeviceObject;
		while (class_device_object && !mouse_obj->service_callback)
		{
			if (!class_device_object->NextDevice && !mouse_obj->mouse_device)
			{
				mouse_obj->mouse_device = class_device_object;
			}

			PULONG_PTR device_extension = (PULONG_PTR)hid_device_object->DeviceExtension;
			ULONG_PTR device_ext_size = ((ULONG_PTR)hid_device_object->DeviceObjectExtension - (ULONG_PTR)hid_device_object->DeviceExtension) / 4;
			class_driver_base = class_driver_object->DriverStart;
			for (ULONG_PTR i = 0; i < device_ext_size; i++)
			{
				if (device_extension[i] == (ULONG_PTR)class_device_object && device_extension[i + 1] > (ULONG_PTR)class_driver_object)
				{
					mouse_obj->service_callback = (MouseClassServiceCallback)(device_extension[i + 1]);
					break;
				}
			}
			class_device_object = class_device_object->NextDevice;
		}
		hid_device_object = hid_device_object->AttachedDevice;
	}

	if (!mouse_obj->mouse_device)
	{
		PDEVICE_OBJECT target_device_object = class_driver_object->DeviceObject;
		while (target_device_object)
		{
			if (!target_device_object->NextDevice)
			{
				mouse_obj->mouse_device = target_device_object;
				break;
			}
			target_device_object = target_device_object->NextDevice;
		}
	}

	ObDereferenceObject(class_driver_object);
	ObDereferenceObject(hid_driver_object);


	return STATUS_SUCCESS;
}


#define MOUSE_LEFT_DOWN   0x0001  // Left Button changed to down.
#define MOUSE_LEFT_UP     0x0002  // Left Button changed to up.
#define MOUSE_RIGHT_DOWN  0x0004  // Right Button changed to down.
#define MOUSE_RIGHT_UP    0x0008  // Right Button changed to up.
#define MOUSE_MIDDLE_DOWN 0x0010  // Middle Button changed to down.
#define MOUSE_MIDDLE_UP   0x0020  // Middle Button changed to up.


void MouseMove(long x, long y)
{
	ULONG input_data;
	MOUSE_INPUT_DATA mid = { 0 };

	mid.LastX = x;
	mid.LastY = y;
	mid.ButtonFlags = 0;
	mid.Flags = MOUSE_MOVE_RELATIVE;


	mouseObject.service_callback(mouseObject.mouse_device, &mid, (PMOUSE_INPUT_DATA)&mid + 1, &input_data);
}

void MouseClick(unsigned short button)
{
	ULONG input_data;
	MOUSE_INPUT_DATA mid = { 0 };

	mid.LastX = 0;
	mid.LastY = 0;
	mid.ButtonFlags = button;
	mid.Flags = MOUSE_MOVE_ABSOLUTE;

	mouseObject.service_callback(mouseObject.mouse_device, &mid, (PMOUSE_INPUT_DATA)&mid + 1, &input_data);
}

