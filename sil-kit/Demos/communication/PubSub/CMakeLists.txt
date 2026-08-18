# SPDX-FileCopyrightText: 2024 Vector Informatik GmbH
#
# SPDX-License-Identifier: MIT

make_silkit_communication_demo(SilKitDemoPublisher PublisherDemo.cpp)
# make_silkit_communication_demo(SilKitDemoSubscriber SubscriberDemo.cpp)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

add_executable(SilKitDemoSubscriber
    SubscriberDemo.cpp
    QtPlotBridge.cpp
    RealTimePlotter.cpp
    SimplePlotWidget.cpp
    PendulumWidget.cpp
)

target_link_libraries(SilKitDemoSubscriber
    PRIVATE
        SilKit::SilKit
        Qt6::Widgets     # or Qt5::Widgets, depending on your Qt version
)

set_target_properties(SilKitDemoSubscriber PROPERTIES
    AUTOMOC ON
)