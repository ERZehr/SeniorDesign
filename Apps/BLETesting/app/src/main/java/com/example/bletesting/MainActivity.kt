package com.example.bletesting

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.Toast
import androidx.annotation.RequiresApi
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import com.example.bletesting.blescanner.BleScanManager
import com.example.bletesting.blescanner.adapter.BleDeviceAdapter
import com.example.bletesting.blescanner.model.BleDevice
import com.example.bletesting.blescanner.model.BleScanCallback
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import com.example.bletesting.permissions.BleScanRequiredPermissions
import com.example.bletesting.permissions.PermissionsUtilities
import com.example.bletesting.permissions.PermissionsUtilities.dispatchOnRequestPermissionsResult
import java.util.UUID

class MainActivity : AppCompatActivity() {
    private lateinit var btnStartScan: Button
    private lateinit var btnConnect: Button
    private lateinit var btnDisconnect: Button
    private lateinit var btnRead: Button
    private lateinit var btManager: BluetoothManager
    private lateinit var bleScanManager: BleScanManager
    private lateinit var foundDevices: MutableList<BleDevice>
    private lateinit var bluetoothAdapter: BluetoothAdapter
    private lateinit var connGatt: BluetoothGatt

    private fun BluetoothGattCharacteristic.isReadable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_READ)

    private fun BluetoothGattCharacteristic.isWritable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE)

    private fun BluetoothGattCharacteristic.isWritableWithoutResponse(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)

    private fun BluetoothGattCharacteristic.containsProperty(property: Int): Boolean {
        return properties and property != 0
    }

    @SuppressLint("MissingPermission")
    private fun readMyChar() {
        val myServUuid = UUID.fromString("00001800-0000-1000-8000-00805f9b34fb")
        val myCharUuid = UUID.fromString("00002a00-0000-1000-8000-00805f9b34fb")
        val myChar = connGatt
            .getService(myServUuid)?.getCharacteristic(myCharUuid)
        //Log.i("readMyChar", "Read characteristic ${myChar}:\n${connGatt.readCharacteristic(myChar)}")
        if (myChar?.isReadable() == true) {
            Log.i("readMyChar", "Read characteristic ${myChar}:\n${connGatt.readCharacteristic(myChar)}")
            connGatt.readCharacteristic(myChar)
        }
        else {
            Log.i("readMyChar", "Not Readable")
        }
    }

    private fun BluetoothGatt.printGattTable() {
        if (services.isEmpty()) {
            Log.i("printGattTable", "No service and characteristic available, call discoverServices() first?")
            return
        }
        services.forEach { service ->
            Log.i("Service Chars:", service.characteristics.toString())
            //val myConn = connGatt.getService(service.uuid)?.getCharacteristic(myCharUuid)
            //if(myConn?.isReadable() == true){

            //}
            val characteristicsTable = service.characteristics.joinToString(
                separator = "\n|--",
                prefix = "|--"
            ) { it.uuid.toString() }
            Log.i("printGattTable", "\nService ${service.uuid}\nCharacteristics:\n$characteristicsTable"
            )
            service.characteristics.forEach { char ->
                val myConn = connGatt.getService(service.uuid)?.getCharacteristic(char.uuid)
                if(myConn?.isReadable() == true){
                    Log.i("Service Chars:", service.uuid.toString() + char.uuid.toString())
                }
            }
        }
    }

    private val gattCallback = object : BluetoothGattCallback() {
        @SuppressLint("MissingPermission")
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            val deviceAddress = gatt.device.address

            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    Log.w("BluetoothGattCallback", "Successfully connected to $deviceAddress")
                    connGatt = gatt
                    Handler(Looper.getMainLooper()).post {
                        gatt.discoverServices()
                    }
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    Log.w("BluetoothGattCallback", "Successfully disconnected from $deviceAddress")
                    gatt.close()
                }
            } else {
                Log.w("BluetoothGattCallback", "Error $status encountered for $deviceAddress! Disconnecting...")
                gatt.close()
            }
        }
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            with(gatt) {
                Log.w("BluetoothGattCallback", "Discovered ${services.size} services for ${device.address}")
                printGattTable() // See implementation just above this section
                // Consider connection setup as complete here
            }
        }
        @SuppressLint("MissingPermission")
        override fun onCharacteristicRead(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            value: ByteArray,
            status: Int
        ) {
            Log.i("BluetoothGattCallback", "Reading")
            with(characteristic) {
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.i("BluetoothGattCallback", "Read characteristic $uuid:\n${value}")
                    }
                    BluetoothGatt.GATT_READ_NOT_PERMITTED -> {
                        Log.e("BluetoothGattCallback", "Read not permitted for $uuid!")
                    }
                    else -> {
                        Log.e("BluetoothGattCallback", "Characteristic read failed for $uuid, error: $status")
                    }
                }
            }
        }
    }

    @SuppressLint("NotifyDataSetChanged", "MissingPermission")
    @RequiresApi(Build.VERSION_CODES.S)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        val MY_UUID = UUID.randomUUID()



        // RecyclerView handling
        val rvFoundDevices = findViewById<View>(R.id.rv_found_devices) as RecyclerView
        foundDevices = BleDevice.createBleDevicesList()
        val adapter = BleDeviceAdapter(foundDevices)
        rvFoundDevices.adapter = adapter
        rvFoundDevices.layoutManager = LinearLayoutManager(this)

        // BleManager creation
        var devAddress = ""
        var devConn: BluetoothDevice? = null
        btManager = getSystemService(BluetoothManager::class.java)
        bleScanManager = BleScanManager(btManager, 5000, scanCallback = BleScanCallback({
            val address = it?.device?.address
            val name = it?.scanRecord?.deviceName + " " + address
            if (address.isNullOrBlank() || it.scanRecord?.deviceName != "ESP32_BLE") return@BleScanCallback
            if (address.isNotBlank()) {
                devAddress = address
                devConn = it.device
            }
            if(it.scanRecord?.deviceName == "ESP32_BLE")
                println("Name: " + it.scanRecord?.deviceName + "Device: " + name) //CC:DB:A7:56:4A:3E

            val device = BleDevice(name)
            if (!foundDevices.contains(device)) {
                foundDevices.add(device)
                adapter.notifyItemInserted(foundDevices.size - 1)
            }
        }))
        bluetoothAdapter = btManager.adapter

        // Adding the actions the manager must do before and after scanning
        bleScanManager.beforeScanActions.add { btnStartScan.isEnabled = false }
        bleScanManager.beforeScanActions.add {
            foundDevices.clear()
            adapter.notifyDataSetChanged()
        }
        bleScanManager.afterScanActions.add { btnStartScan.isEnabled = true }

        // Adding the onclick listener to the start scan button

        btnStartScan = findViewById(R.id.btn_start_scan)
        btnStartScan.setOnClickListener {
            // Checks if the required permissions are granted and starts the scan if so, otherwise it requests them
            println("Clicked")
            when (PermissionsUtilities.checkPermissionsGranted(
                this,
                BleScanRequiredPermissions.permissions
            )) {
                true -> bleScanManager.scanBleDevices()
                false -> {
                    PermissionsUtilities.checkPermissions(
                        this, BleScanRequiredPermissions.permissions, BLE_PERMISSION_REQUEST_CODE
                    )
                    println("Needs permissions")
                }
            }
        }
        btnConnect = findViewById(R.id.btn_connect)
        btnConnect .setOnClickListener {
            println("Device Address: $devAddress")
            Log.w("btnConnect", "Connecting to ${devConn?.address}")
            devConn?.connectGatt(this, false, gattCallback)

            println("Connected")
        }
        btnRead = findViewById(R.id.btn_read)
        btnRead .setOnClickListener {
            Log.w("btnRead", "Reading from ${devConn?.address}")
            readMyChar()
        }
        btnDisconnect = findViewById(R.id.btn_disconnect)
        btnDisconnect .setOnClickListener {
            Log.w("btnDisconnect", "Disconnecting from ${devConn?.address}")
            connGatt.disconnect();

            println("Disconnected")
        }
    }

    @RequiresApi(Build.VERSION_CODES.S)
    override fun onRequestPermissionsResult(
        requestCode: Int, permissions: Array<out String>, grantResults: IntArray
    ) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)

        dispatchOnRequestPermissionsResult(
            requestCode,
            grantResults,
            onGrantedMap = mapOf(BLE_PERMISSION_REQUEST_CODE to { bleScanManager.scanBleDevices() }),
            onDeniedMap = mapOf(BLE_PERMISSION_REQUEST_CODE to { Toast.makeText(this,
                "Some permissions were not granted, please grant them and try again",
                Toast.LENGTH_LONG).show() })
        )
    }

    companion object { private const val BLE_PERMISSION_REQUEST_CODE = 1 }
}