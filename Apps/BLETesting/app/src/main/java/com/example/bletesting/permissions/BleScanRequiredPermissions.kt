package com.example.bletesting.permissions

import android.Manifest
import android.os.Build
import androidx.annotation.RequiresApi


object BleScanRequiredPermissions {
    @RequiresApi(Build.VERSION_CODES.S)
    val permissions = arrayOf(
        Manifest.permission.ACCESS_COARSE_LOCATION,
        Manifest.permission.ACCESS_FINE_LOCATION,
        Manifest.permission.BLUETOOTH_ADMIN,
    )
}