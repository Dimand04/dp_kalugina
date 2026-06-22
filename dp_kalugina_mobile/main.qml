import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import AppComponents 1.0

ApplicationWindow {
    id: window
    width: 360
    height: 640
    visible: true
    title: qsTr("Склад ТСД")

    property string serverIp: "192.168.1.101"
    property int serverPort: 54321

    ListModel {
        id: materialsModel
    }

    ListModel {
        id: warehouseModel
    }

    ListModel {
        id: scannedItemsModel
    }

    Connections {
        target: netManager

        function onMaterialsReceived(jsonData) {
            materialsModel.clear()
            var materialsArray = JSON.parse(jsonData)
            for (var i = 0; i < materialsArray.length; i++) {
                var item = materialsArray[i]
                materialsModel.append({
                    "matId": item.id,
                    "matName": item.name
                })
            }
        }

        function onWarehouseReceived(jsonData) {
            warehouseModel.clear()
            var array = JSON.parse(jsonData)
            for (var i = 0; i < array.length; i++) {
                var item = array[i]
                warehouseModel.append({
                    "wId": item.material_id,
                    "wCategory": item.category,
                    "wName": item.name,
                    "wQty": item.quantity,
                    "wUnit": item.unit,
                    "wPrice": item.avg_price,
                    "wSum": item.total_sum
                })
            }
        }

        function onInventorySentSuccess() {
            scannedItemsModel.clear()
            popupText.text = "Инвентаризация успешно отправлена и сохранена на ПК!"
            msgPopup.open()
        }

        function onErrorOccurred(errorMsg) {
            popupText.text = "ОШИБКА:\n" + errorMsg
            msgPopup.open()
        }
    }

    header: ToolBar {
        Label {
            text: {
                switch(swipeView.currentIndex) {
                    case 0: return "Инвентаризация"
                    case 1: return "База материалов"
                    case 2: return "Текущие остатки"
                    case 3: return "Настройки сети"
                }
            }
            font.pixelSize: 20
            anchors.centerIn: parent
            font.bold: true
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        // ЭКРАН 1: ИНВЕНТАРИЗАЦИЯ СО СКАНЕРОМ (Адаптировано для Qt 5)
        Page {
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 10
                        spacing: 10

                        // 1. Блок камеры и сканера
                        Rectangle {
                            Layout.alignment: Qt.AlignHCenter
                            Layout.preferredWidth: parent.width * 0.7
                            Layout.preferredHeight: parent.width * 0.7
                            color: "black"
                            radius: 10
                            clip: true

                            // Сессия захвата Qt 6
                            CaptureSession {
                                                    camera: Camera {
                                                        id: camera
                                                        active: swipeView.currentIndex === 0
                                                        focusMode: Camera.FocusModeAuto // <--- ИСПРАВЛЕНО
                                                    }
                                                    videoOutput: videoOutput
                                                }

                            // Вывод видео на экран
                            VideoOutput {
                                id: videoOutput
                                anchors.fill: parent
                                fillMode: VideoOutput.PreserveAspectCrop
                            }

                            // Наш самописный C++ сканер!
                            BarcodeScanner {
                                id: scanner
                                // Передаем ему поток кадров прямо из экрана
                                videoSink: videoOutput.videoSink
                                active: swipeView.currentIndex === 0

                                // Когда C++ найдет код, сработает этот сигнал
                                onBarcodeDecoded: function(text) {
                                    scanResultField.text = text
                                    quantityField.forceActiveFocus()
                                    scanner.active = false // Ставим на паузу
                                    resetTimer.start()
                                }
                            }

                            Timer {
                                id: resetTimer
                                interval: 2000
                                onTriggered: scanner.active = true // Снимаем с паузы
                            }
                        }

                // 2. Строка с отсканированным кодом
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: scanResultField
                        Layout.fillWidth: true
                        placeholderText: "ID / Штрихкод"
                    }
                    Button {
                        text: "Сброс"
                        onClicked: {
                            scanResultField.text = ""
                            zxingFilter.active = true
                        }
                    }
                }

                // 3. Строка количества и кнопка "Добавить"
                RowLayout {
                    Layout.fillWidth: true
                    TextField {
                        id: quantityField
                        Layout.fillWidth: true
                        placeholderText: "Фактическое кол-во"
                        inputMethodHints: Qt.ImhFormattedNumbersOnly
                    }
                    Button {
                        text: "Добавить"
                        onClicked: {
                            if (scanResultField.text !== "" && quantityField.text !== "") {
                                scannedItemsModel.append({
                                    "matId": parseInt(scanResultField.text),
                                    "actualQty": parseFloat(quantityField.text.replace(',', '.'))
                                })
                                scanResultField.text = ""
                                quantityField.text = ""
                                zxingFilter.active = true // Снова включаем сканер
                            }
                        }
                    }
                }

                // 4. Список добавленных позиций
                ListView {
                    id: scannedListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    model: scannedItemsModel
                    spacing: 5

                    delegate: Rectangle {
                        width: scannedListView.width
                        height: 50
                        color: "#e3f2fd"
                        radius: 5
                        border.color: "#bbdefb"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10

                            Text {
                                text: "ID: " + model.matId
                                font.bold: true
                                color: "#1565c0"
                                Layout.minimumWidth: 60
                            }

                            Text {
                                text: "Факт: " + model.actualQty
                                Layout.fillWidth: true
                                font.pixelSize: 16
                            }

                            Button {
                                text: "X"
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                onClicked: scannedItemsModel.remove(index)
                            }
                        }
                    }
                }

                // 5. Отправка на ПК
                Button {
                    text: "Отправить данные на ПК"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    Material.background: Material.Green

                    onClicked: {
                        if (scannedItemsModel.count === 0) return;

                        var dataArray = []
                        for (var i = 0; i < scannedItemsModel.count; i++) {
                            dataArray.push({
                                "material_id": scannedItemsModel.get(i).matId,
                                "actual_quantity": scannedItemsModel.get(i).actualQty
                            })
                        }

                        var jsonString = JSON.stringify(dataArray)
                        netManager.sendInventory(window.serverIp, window.serverPort, jsonString)
                    }
                }
            }
        }

        // ЭКРАН 2: БАЗА МАТЕРИАЛОВ
        Page {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10

                Button {
                    text: "Обновить список"
                    Layout.fillWidth: true
                    onClicked: netManager.fetchMaterials(window.serverIp, window.serverPort)
                }

                ListView {
                    id: materialsListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 5

                    model: materialsModel

                    delegate: Rectangle {
                        width: materialsListView.width
                        height: 50
                        color: "#f0f0f0"
                        radius: 5
                        border.color: "#cccccc"
                        border.width: 1

                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 10

                            Text {
                                text: "ID: " + model.matId
                                font.bold: true
                                color: "#444444"
                                Layout.minimumWidth: 40
                            }

                            Text {
                                text: model.matName
                                font.pixelSize: 16
                                Layout.fillWidth: true
                                wrapMode: Text.WordWrap
                            }
                        }
                    }
                }
            }
        }

        // ЭКРАН 3: ТЕКУЩИЕ ОСТАТКИ
        Page {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10

                Button {
                    text: "Обновить остатки"
                    Layout.fillWidth: true
                    onClicked: netManager.fetchWarehouse(window.serverIp, window.serverPort)
                }

                ListView {
                    id: warehouseListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 10
                    model: warehouseModel

                    delegate: Rectangle {
                        width: warehouseListView.width
                        height: 90
                        color: "#ffffff"
                        radius: 8
                        border.color: "#cccccc"
                        border.width: 1

                        ColumnLayout {
                            anchors.fill: parent
                            anchors.margins: 10
                            spacing: 2

                            Text {
                                text: model.wName
                                font.bold: true
                                font.pixelSize: 16
                                color: "#222222"
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Text {
                                text: model.wCategory
                                font.pixelSize: 12
                                color: "#777777"
                                Layout.fillWidth: true
                            }

                            RowLayout {
                                Layout.fillWidth: true
                                Layout.topMargin: 5

                                Text {
                                    text: "В наличии: " + Number(model.wQty).toLocaleString(Qt.locale(), 'f', 2) + " " + model.wUnit
                                    font.pixelSize: 14
                                    color: "#006600"
                                    Layout.alignment: Qt.AlignLeft
                                }

                                Item { Layout.fillWidth: true }

                                Text {
                                    text: "Сумма: " + Number(model.wSum).toLocaleString(Qt.locale(), 'f', 2) + " ₽"
                                    font.bold: true
                                    font.pixelSize: 14
                                    color: "#333333"
                                    Layout.alignment: Qt.AlignRight
                                }
                            }
                        }
                    }
                }
            }
        }

        // ЭКРАН 4: НАСТРОЙКИ СЕТИ
        Page {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 15
                width: parent.width * 0.8

                Text { text: "IP-адрес сервера:"; font.bold: true }
                TextField {
                    id: ipInput
                    Layout.fillWidth: true
                    text: window.serverIp
                }

                Text { text: "Порт:"; font.bold: true }
                TextField {
                    id: portInput
                    Layout.fillWidth: true
                    text: window.serverPort.toString()
                    inputMethodHints: Qt.ImhDigitsOnly
                }

                Button {
                    text: "Сохранить настройки"
                    Layout.fillWidth: true
                    onClicked: {
                        window.serverIp = ipInput.text
                        window.serverPort = parseInt(portInput.text)
                    }
                }
            }
        }
    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        width: parent.width

        TabButton {
            text: qsTr("Скан")
        }
        TabButton {
            text: qsTr("База")
        }
        TabButton {
            text: qsTr("Склад")
        }
        TabButton {
            text: qsTr("Настройки")
        }
    }

    Popup {
        id: msgPopup
        anchors.centerIn: parent
        width: 250
        height: 100
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#ffffff"
            radius: 8
            border.color: "#cccccc"
        }

        contentItem: Text {
            id: popupText
            text: ""
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font.pixelSize: 14
        }
    }
}
