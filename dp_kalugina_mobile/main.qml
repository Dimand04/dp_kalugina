import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtMultimedia
import AppComponents 1.0
import QtCore // Добавлено для сохранения настроек

ApplicationWindow {
    id: window
    width: 360
    height: 640
    visible: true
    title: qsTr("Склад ТСД")

    // Сохранение настроек в память устройства
    Settings {
        id: appSettings
        property string serverIp: "192.168.1.101"
        property int serverPort: 54321
    }

    ListModel { id: materialsModel }
    ListModel { id: warehouseModel }
    ListModel { id: scannedItemsModel }

    // Универсальная функция для показа красивых уведомлений
    function showPopup(title, msg, isError) {
        popupTitle.text = title
        popupText.text = msg
        popupTitle.color = isError ? "#d32f2f" : "#2e7d32" // Красный заголовок для ошибок, зеленый для успеха
        msgPopup.open()
    }

    Connections {
        target: netManager

        function onMaterialsReceived(jsonData) {
            materialsModel.clear()
            var array = JSON.parse(jsonData)
            for (var i = 0; i < array.length; i++) {
                materialsModel.append({"matId": array[i].id, "matName": array[i].name})
            }
            showPopup("Успех", "База материалов загружена (" + array.length + " позиций).", false)
        }

        function onWarehouseReceived(jsonData) {
            warehouseModel.clear()
            var array = JSON.parse(jsonData)
            for (var i = 0; i < array.length; i++) {
                warehouseModel.append({
                    "wId": array[i].material_id, "wCategory": array[i].category, "wName": array[i].name,
                    "wQty": array[i].quantity, "wUnit": array[i].unit, "wPrice": array[i].avg_price, "wSum": array[i].total_sum
                })
            }
            showPopup("Успех", "Текущие остатки обновлены.", false)
        }

        function onInventorySentSuccess() {
            scannedItemsModel.clear()
            showPopup("Отправлено", "Инвентаризация успешно сохранена в базе на ПК!", false)
        }

        function onErrorOccurred(errorMsg) {
            showPopup("Ошибка сети", errorMsg, true)
        }
    }

    header: ToolBar {
        Label {
            text: ["Инвентаризация", "База материалов", "Остатки", "Настройки"][swipeView.currentIndex]
            font.pixelSize: 20
            anchors.centerIn: parent
            font.bold: true
        }
    }

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: tabBar.currentIndex

        // ЭКРАН 1: ИНВЕНТАРИЗАЦИЯ
        Page {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                spacing: 10

                Rectangle {
                    Layout.alignment: Qt.AlignHCenter
                    Layout.preferredWidth: parent.width * 0.7
                    Layout.preferredHeight: parent.width * 0.7
                    color: "black"
                    radius: 10
                    clip: true

                    CaptureSession {
                        camera: Camera {
                            id: camera
                            active: swipeView.currentIndex === 0
                            focusMode: Camera.FocusModeAuto
                        }
                        videoOutput: videoOutput
                    }

                    VideoOutput {
                        id: videoOutput
                        anchors.fill: parent
                        fillMode: VideoOutput.PreserveAspectCrop
                    }

                    BarcodeScanner {
                        id: scanner
                        videoSink: videoOutput.videoSink
                        active: swipeView.currentIndex === 0

                        onBarcodeDecoded: function(text) {
                            scanResultField.text = text
                            quantityField.forceActiveFocus()
                            scanner.active = false
                            resetTimer.start()
                        }
                    }

                    Timer {
                        id: resetTimer
                        interval: 2000
                        onTriggered: scanner.active = true
                    }
                }

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
                            scanner.active = true
                        }
                    }
                }

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
                            // ЗАЩИТА: Пустые поля
                            if (scanResultField.text === "") {
                                showPopup("Внимание", "Отсканируйте или введите ID материала!", true)
                                return;
                            }
                            if (quantityField.text === "") {
                                showPopup("Внимание", "Введите фактическое количество!", true)
                                return;
                            }

                            // ЗАЩИТА: Запятые и нули
                            var qty = parseFloat(quantityField.text.replace(',', '.'))
                            if (isNaN(qty) || qty <= 0) {
                                showPopup("Ошибка ввода", "Количество должно быть больше нуля!", true)
                                return;
                            }

                            scannedItemsModel.append({
                                "matId": parseInt(scanResultField.text),
                                "actualQty": qty
                            })
                            scanResultField.text = ""
                            quantityField.text = ""
                            scanner.active = true
                        }
                    }
                }

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
                            Text { text: "ID: " + model.matId; font.bold: true; color: "#1565c0"; Layout.minimumWidth: 60 }
                            Text { text: "Факт: " + model.actualQty; Layout.fillWidth: true; font.pixelSize: 16 }
                            Button {
                                text: "X"
                                Layout.preferredWidth: 40
                                Layout.preferredHeight: 40
                                onClicked: scannedItemsModel.remove(index)
                            }
                        }
                    }
                }

                Button {
                    text: "Отправить данные на ПК"
                    Layout.fillWidth: true
                    Layout.preferredHeight: 50
                    Material.background: Material.Green

                    onClicked: {
                        // ЗАЩИТА: Отправка пустого списка
                        if (scannedItemsModel.count === 0) {
                            showPopup("Отмена", "Список инвентаризации пуст. Добавьте материалы.", true)
                            return;
                        }

                        var dataArray = []
                        for (var i = 0; i < scannedItemsModel.count; i++) {
                            dataArray.push({
                                "material_id": scannedItemsModel.get(i).matId,
                                "actual_quantity": scannedItemsModel.get(i).actualQty
                            })
                        }
                        netManager.sendInventory(appSettings.serverIp, appSettings.serverPort, JSON.stringify(dataArray))
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
                    onClicked: netManager.fetchMaterials(appSettings.serverIp, appSettings.serverPort)
                }
                ListView {
                    id: materialsListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 5
                    model: materialsModel
                    delegate: Rectangle {
                        width: materialsListView.width; height: 50; color: "#f0f0f0"; radius: 5; border.color: "#cccccc"
                        RowLayout {
                            anchors.fill: parent; anchors.margins: 10
                            Text { text: "ID: " + model.matId; font.bold: true; color: "#444444"; Layout.minimumWidth: 40 }
                            Text { text: model.matName; font.pixelSize: 16; Layout.fillWidth: true; wrapMode: Text.WordWrap }
                        }
                    }
                }
            }
        }

        // ЭКРАН 3: ОСТАТКИ
        Page {
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 10
                Button {
                    text: "Обновить остатки"
                    Layout.fillWidth: true
                    onClicked: netManager.fetchWarehouse(appSettings.serverIp, appSettings.serverPort)
                }
                ListView {
                    id: warehouseListView
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    clip: true
                    spacing: 10
                    model: warehouseModel
                    delegate: Rectangle {
                        width: warehouseListView.width; height: 90; color: "#ffffff"; radius: 8; border.color: "#cccccc"
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 10; spacing: 2
                            Text { text: model.wName; font.bold: true; font.pixelSize: 16; Layout.fillWidth: true; elide: Text.ElideRight }
                            Text { text: model.wCategory; font.pixelSize: 12; color: "#777777"; Layout.fillWidth: true }
                            RowLayout {
                                Layout.fillWidth: true; Layout.topMargin: 5
                                Text { text: "В наличии: " + Number(model.wQty).toLocaleString(Qt.locale(), 'f', 2) + " " + model.wUnit; font.pixelSize: 14; color: "#006600"; Layout.alignment: Qt.AlignLeft }
                                Item { Layout.fillWidth: true }
                                Text { text: "Сумма: " + Number(model.wSum).toLocaleString(Qt.locale(), 'f', 2) + " ₽"; font.bold: true; font.pixelSize: 14; Layout.alignment: Qt.AlignRight }
                            }
                        }
                    }
                }
            }
        }

        // ЭКРАН 4: НАСТРОЙКИ (с сохранением)
        Page {
            ColumnLayout {
                anchors.centerIn: parent
                spacing: 15
                width: parent.width * 0.8

                Text { text: "IP-адрес сервера:"; font.bold: true }
                TextField {
                    id: ipInput
                    Layout.fillWidth: true
                    text: appSettings.serverIp // Подтягиваем из памяти
                }

                Text { text: "Порт:"; font.bold: true }
                TextField {
                    id: portInput
                    Layout.fillWidth: true
                    text: appSettings.serverPort.toString()
                    inputMethodHints: Qt.ImhDigitsOnly
                }

                Button {
                    text: "Сохранить настройки"
                    Layout.fillWidth: true
                    Material.background: Material.Primary
                    onClicked: {
                        // ЗАЩИТА: пустые поля
                        if (ipInput.text === "" || portInput.text === "") {
                            showPopup("Ошибка", "Заполните оба поля!", true)
                            return;
                        }

                        // Сохраняем в память устройства
                        appSettings.serverIp = ipInput.text
                        appSettings.serverPort = parseInt(portInput.text)

                        showPopup("Настройки", "Данные сервера успешно сохранены.", false)
                    }
                }
            }
        }
    }

    footer: TabBar {
        id: tabBar
        currentIndex: swipeView.currentIndex
        width: parent.width

        TabButton { text: qsTr("Скан") }
        TabButton { text: qsTr("База") }
        TabButton { text: qsTr("Склад") }
        TabButton { text: qsTr("Настройки") }
    }

    // УНИВЕРСАЛЬНОЕ ВСПЛЫВАЮЩЕЕ ОКНО С ЗАГОЛОВКОМ
    Popup {
        id: msgPopup
        anchors.centerIn: parent
        width: parent.width * 0.85
        height: 140
        modal: true
        focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside

        background: Rectangle {
            color: "#ffffff"
            radius: 8
            border.color: "#cccccc"
            border.width: 1
        }

        contentItem: ColumnLayout {
            spacing: 10
            Text {
                id: popupTitle
                text: "Заголовок"
                font.bold: true
                font.pixelSize: 18
                Layout.alignment: Qt.AlignHCenter
            }
            Text {
                id: popupText
                text: "Описание ошибки или успеха"
                wrapMode: Text.WordWrap
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
                font.pixelSize: 14
            }
            Button {
                text: "ОК"
                Layout.alignment: Qt.AlignHCenter
                onClicked: msgPopup.close()
            }
        }
    }
}
