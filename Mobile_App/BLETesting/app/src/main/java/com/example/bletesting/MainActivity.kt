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
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import android.widget.AdapterView
import android.widget.ArrayAdapter
import android.widget.Button
import android.widget.SeekBar
import android.widget.Spinner
import android.widget.TextView
import android.widget.Toast
import androidx.activity.result.contract.ActivityResultContracts
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
import kotlin.properties.Delegates

class MainActivity : AppCompatActivity() {
    private lateinit var btnStartScan: Button
    private lateinit var btnConnect: Button
    private lateinit var btnMode: Button
    private lateinit var btnWrite: Button
    private lateinit var btManager: BluetoothManager
    private lateinit var bleScanManager: BleScanManager
    private lateinit var bluetoothAdapter: BluetoothAdapter
    private lateinit var foundDevices: MutableList<BleDevice>
    private lateinit var connGatt: BluetoothGatt
    private var dispMode = 0
    private var colorMode = 0

    private fun BluetoothGattCharacteristic.isReadable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_READ)

    private fun BluetoothGattCharacteristic.isWritable(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE)

    private fun BluetoothGattCharacteristic.isWritableWithoutResponse(): Boolean =
        containsProperty(BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE)

    private fun BluetoothGattCharacteristic.containsProperty(property: Int): Boolean {
        return properties and property != 0
    }



    private fun getChar(): BluetoothGattCharacteristic? {
        val myServUuid = UUID.fromString("6e400001-b5a3-f393-e0a9-e50e24dcca9e")
        val myCharUuid = UUID.fromString("6e400002-b5a3-f393-e0a9-e50e24dcca9e")
        return connGatt
            .getService(myServUuid)?.getCharacteristic(myCharUuid)
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


    @SuppressLint("MissingPermission")
    private fun writeMyCharacteristic(gatt: BluetoothGatt, characteristic: BluetoothGattCharacteristic, data: ByteArray) {
        // Check if the characteristic is writable
        if (characteristic.properties and BluetoothGattCharacteristic.PROPERTY_WRITE != 0 ||
            characteristic.properties and BluetoothGattCharacteristic.PROPERTY_WRITE_NO_RESPONSE != 0) {

            // Set the value of the characteristic
            characteristic.value = data

            // Write the characteristic
            gatt.writeCharacteristic(characteristic)
        } else {
            // Characteristic is not writable

        }
    }

    private fun BluetoothGatt.printGattTable() {
        if (services.isEmpty()) {
            Log.i("printGattTable", "No service and characteristic available, call discoverServices() first?")
            return
        }
        services.forEach { service ->
            val characteristicsTable = service.characteristics.joinToString(
                separator = "\n|--",
                prefix = "|--"
            ) { it.uuid.toString() }
            Log.i("printGattTable", "\nService ${service.uuid}\nCharacteristics:\n$characteristicsTable"
            )
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
            status: Int
        ) {
            with(characteristic) {
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.i("BluetoothGattCallback", "Read characteristic $uuid:\n${String(value)}")
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
        override fun onCharacteristicWrite(
            gatt: BluetoothGatt,
            characteristic: BluetoothGattCharacteristic,
            status: Int
        ) {
            with(characteristic) {
                when (status) {
                    BluetoothGatt.GATT_SUCCESS -> {
                        Log.i("BluetoothGattCallback", "Wrote to characteristic $uuid | value: ${String(characteristic.value)}")
                    }
                    BluetoothGatt.GATT_INVALID_ATTRIBUTE_LENGTH -> {
                        Log.e("BluetoothGattCallback", "Write exceeded connection ATT MTU!")
                    }
                    BluetoothGatt.GATT_WRITE_NOT_PERMITTED -> {
                        Log.e("BluetoothGattCallback", "Write not permitted for $uuid!")
                    }
                    else -> {
                        Log.e("BluetoothGattCallback", "Characteristic write failed for $uuid, error: $status")
                    }
                }
            }
        }
    }

    @SuppressLint("NotifyDataSetChanged", "MissingPermission")
    @RequiresApi(Build.VERSION_CODES.S, Build.VERSION_CODES.TIRAMISU)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        foundDevices = BleDevice.createBleDevicesList()
        val adapter = BleDeviceAdapter(foundDevices)

        // BleManager creation
        var devAddress = ""
        var devConn: BluetoothDevice? = null
        var devName = ""
        val labelScan: TextView  = findViewById(R.id.labelScan)
        btManager = getSystemService(BluetoothManager::class.java)
        bleScanManager = BleScanManager(btManager, 5000, scanCallback = BleScanCallback({ it ->
            val address = it?.device?.address
            val name = it?.scanRecord?.deviceName + " " + address
            if (address.isNullOrBlank() || it.scanRecord?.deviceName != "ARTISYN_BLE") return@BleScanCallback
            if(address.isNotBlank() && it.scanRecord?.deviceName == "ARTISYN_BLE") {
                devName = "ARTISYN_BLE"
                "Found Artisyn Device!".also { labelScan.text = it }
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
        val sliderQ: SeekBar = findViewById(R.id.sliderQ)
        val sliderFreq: SeekBar = findViewById(R.id.sliderFreq)
        val sliderBands: SeekBar = findViewById(R.id.sliderBands)
        val labelFreq: TextView  = findViewById(R.id.labelFreq)
        val labelQ: TextView  = findViewById(R.id.labelQ)
        val labelBands: TextView  = findViewById(R.id.labelBands)

        sliderQ.setOnSeekBarChangeListener(object :
            SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seek: SeekBar,
                                           progress: Int, fromUser: Boolean) {
                "Q Factor: ${(progress + 25)/100.0}".also { labelQ.text = it };
            }

            override fun onStartTrackingTouch(seek: SeekBar) {
                // write custom code for progress is started
            }

            override fun onStopTrackingTouch(seek: SeekBar) {
                // write custom code for progress is stopped
            }
        })

        sliderFreq.setOnSeekBarChangeListener(object :
            SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seek: SeekBar,
                                           progress: Int, fromUser: Boolean) {
                "Center Frequency: ${progress * 100}".also { labelFreq.text = it };
            }

            override fun onStartTrackingTouch(seek: SeekBar) {
                // write custom code for progress is started
            }

            override fun onStopTrackingTouch(seek: SeekBar) {
                // write custom code for progress is stopped
            }
        })

        sliderBands.setOnSeekBarChangeListener(object :
            SeekBar.OnSeekBarChangeListener {
            override fun onProgressChanged(seek: SeekBar,
                                           progress: Int, fromUser: Boolean) {
                "Bands: ${(progress + 2)}".also { labelBands.text = it };
            }

            override fun onStartTrackingTouch(seek: SeekBar) {
                // write custom code for progress is started
            }

            override fun onStopTrackingTouch(seek: SeekBar) {
                // write custom code for progress is stopped
            }
        })

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
        val labelConn : TextView = findViewById(R.id.labelConn)
        btnConnect .setOnClickListener {
            println("Device Address: $devAddress")
            if(devConn != null) {
                if (btnConnect.text == "Connect") {
                    Log.w("btnConnect", "Connecting to ${devConn?.address}")
                    devConn?.connectGatt(this, false, gattCallback)
                    "Connected to ${devName}".also { labelConn.text = it }
                    "Disconnect".also { btnConnect.text = it }
                } else if (btnConnect.text == "Disconnect") {
                    Log.w("btnDisconnect", "Disconnecting from ${devConn?.address}")
                    connGatt.disconnect();
                    "Disconnected".also { labelConn.text = it }
                    "Connect".also { btnConnect.text = it }
                    devConn = null
                    "No device".also {labelScan.text = it}
                }
            }
            else {
                "No device found".also { labelConn.text = it }
            }
        }

        btnWrite = findViewById(R.id.btn_write)
        val labelWrite: TextView = findViewById(R.id.labelWrite)
        btnWrite .setOnClickListener {
            if(devConn != null) {
                val myChar = getChar()
                val qFac = (sliderQ.progress + 25) / 100.0
                val centFreq = (sliderFreq.progress * 100)
                val bands = sliderBands.progress + 2

                Log.i("btnWrite", "Center Frequency: $centFreq")
                Log.i("btnWrite", "Q Factor: $qFac")
                val myStr =
                    "{\"USER\" : {\"Q\" : $qFac, \"FC\" : ${centFreq + 63}, \"BANDS\" : $bands}, \"IDLE MODE\" : $dispMode, \"THEME\" : $colorMode}"
                val payload = myStr.toByteArray()
                val string = String(payload)
                "Q: $qFac CF: $centFreq".also { labelWrite.text = it }

                Log.w(
                    "btnWrite",
                    "Writing to ${devConn?.address}| value $string | payload $payload"
                )
                if (myChar != null) {
                    writeMyCharacteristic(connGatt, myChar, payload)
                }
            }
            else {
                "No Connection".also { labelWrite.text = it }
            }
        }

        btnMode = findViewById(R.id.btn_mode)
        val labelMode: TextView = findViewById(R.id.labelMode)
        btnMode .setOnClickListener {


            val qFac = (sliderQ.progress + 25) / 100.0
            val centFreq = sliderFreq.progress * 100
            val bands = sliderBands.progress + 2
            if (dispMode == 0) {
                dispMode = 1
                "Idle".also { labelMode.text = it }
            } else {
                dispMode = 0
                "Active".also { labelMode.text = it }
            }
            if(devConn != null) {
                val myChar = getChar()
                val myStr =
                    "{\"USER\" : {\"Q\" : ${qFac}, \"FC\" : ${centFreq + 63}, \"BANDS\" : $bands}, \"IDLE MODE\" : $dispMode, \"THEME\" : $colorMode}"
                val payload = myStr.toByteArray()
                val string = String(payload)
                "Q: $qFac CF: $centFreq".also { labelWrite.text = it }

                Log.w("btnMode", "Writing to ${devConn?.address}| value $string | payload $payload")
                if (myChar != null) {
                    writeMyCharacteristic(connGatt, myChar, payload)
                }
            }
        }

        val colors = resources.getStringArray(R.array.colors)
        val spinner: Spinner = findViewById(R.id.labelColor)
        if (spinner != null) {
            val spinAdapter = ArrayAdapter(
                this,
                android.R.layout.simple_spinner_item, colors
            )
            spinner.adapter = spinAdapter
        }
        spinner.onItemSelectedListener = object :
            AdapterView.OnItemSelectedListener {
            override fun onItemSelected(parent: AdapterView<*>,
                                        view: View, position: Int, id: Long) {
                Log.i("Spinner", "Color: ${colors[position]}; Position: $position")
                colorMode = position
            }

            override fun onNothingSelected(parent: AdapterView<*>) {
                // write code to perform some action
            }
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