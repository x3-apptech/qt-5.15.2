// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/presentation/presentation_receiver.h"

#include "third_party/blink/public/common/browser_interface_broker_proxy.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise.h"
#include "third_party/blink/renderer/bindings/core/v8/script_promise_resolver.h"
#include "third_party/blink/renderer/core/dom/document.h"
#include "third_party/blink/renderer/core/dom/dom_exception.h"
#include "third_party/blink/renderer/core/execution_context/execution_context.h"
#include "third_party/blink/renderer/core/frame/deprecation.h"
#include "third_party/blink/renderer/core/frame/local_dom_window.h"
#include "third_party/blink/renderer/core/frame/local_frame.h"
#include "third_party/blink/renderer/core/frame/local_frame_client.h"
#include "third_party/blink/renderer/core/frame/navigator.h"
#include "third_party/blink/renderer/modules/presentation/navigator_presentation.h"
#include "third_party/blink/renderer/modules/presentation/presentation.h"
#include "third_party/blink/renderer/modules/presentation/presentation_connection.h"
#include "third_party/blink/renderer/modules/presentation/presentation_connection_list.h"
#include "third_party/blink/renderer/platform/instrumentation/use_counter.h"

namespace blink {

PresentationReceiver::PresentationReceiver(LocalFrame* frame)
    : connection_list_(
          MakeGarbageCollected<PresentationConnectionList>(frame->DomWindow())),
      presentation_receiver_receiver_(this, frame->DomWindow()),
      presentation_service_remote_(frame->DomWindow()),
      frame_(frame) {
  scoped_refptr<base::SingleThreadTaskRunner> task_runner =
      frame->GetTaskRunner(TaskType::kPresentation);
  frame->GetBrowserInterfaceBroker().GetInterface(
      presentation_service_remote_.BindNewPipeAndPassReceiver(task_runner));

  // Set the mojo::Remote<T> that remote implementation of PresentationService
  // will use to interact with the associated PresentationReceiver, in order
  // to receive updates on new connections becoming available.
  presentation_service_remote_->SetReceiver(
      presentation_receiver_receiver_.BindNewPipeAndPassRemote(task_runner));
}

// static
PresentationReceiver* PresentationReceiver::From(Document& document) {
  if (!document.IsInMainFrame() || !document.GetFrame()->DomWindow())
    return nullptr;
  Navigator& navigator = *document.GetFrame()->DomWindow()->navigator();
  Presentation* presentation = NavigatorPresentation::presentation(navigator);
  if (!presentation)
    return nullptr;

  return presentation->receiver();
}

ScriptPromise PresentationReceiver::connectionList(ScriptState* script_state) {
  if (!connection_list_property_) {
    connection_list_property_ = MakeGarbageCollected<ConnectionListProperty>(
        ExecutionContext::From(script_state));
  }

  if (!connection_list_->IsEmpty() &&
      connection_list_property_->GetState() == ConnectionListProperty::kPending)
    connection_list_property_->Resolve(connection_list_);

  return connection_list_property_->Promise(script_state->World());
}

void PresentationReceiver::Terminate() {
  if (!GetFrame())
    return;

  auto* window = GetFrame()->DomWindow();
  if (!window || window->closed())
    return;

  window->Close(window);
}

void PresentationReceiver::RemoveConnection(
    ReceiverPresentationConnection* connection) {
  DCHECK(connection_list_);
  connection_list_->RemoveConnection(connection);
}

void PresentationReceiver::OnReceiverConnectionAvailable(
    mojom::blink::PresentationInfoPtr info,
    mojo::PendingRemote<mojom::blink::PresentationConnection>
        controller_connection,
    mojo::PendingReceiver<mojom::blink::PresentationConnection>
        receiver_connection_receiver) {
  // Take() will call PresentationReceiver::registerConnection()
  // and register the connection.
  auto* connection = ReceiverPresentationConnection::Take(
      this, *info, std::move(controller_connection),
      std::move(receiver_connection_receiver));

  // Only notify receiver.connectionList property if it has been acccessed
  // previously.
  if (!connection_list_property_)
    return;

  if (connection_list_property_->GetState() ==
      ConnectionListProperty::kPending) {
    connection_list_property_->Resolve(connection_list_);
  } else if (connection_list_property_->GetState() ==
             ConnectionListProperty::kResolved) {
    connection_list_->DispatchConnectionAvailableEvent(connection);
  }
}

void PresentationReceiver::RegisterConnection(
    ReceiverPresentationConnection* connection) {
  DCHECK(connection_list_);
  connection_list_->AddConnection(connection);
}

void PresentationReceiver::Trace(Visitor* visitor) {
  visitor->Trace(connection_list_);
  visitor->Trace(connection_list_property_);
  visitor->Trace(presentation_receiver_receiver_);
  visitor->Trace(presentation_service_remote_);
  visitor->Trace(frame_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
