# Deployment

# Native

For the native usage, no special deployment is needed. You can run and test the app as specified in [link](../building/README.md) (native part).

# Container

If you want to run the SUA as a service which can receive update requests any time the following [yaml](sua-service.yaml) is available.

Apply the yaml config with...

```
kubectl apply -f sua-service.yaml
```

The yaml mounts system_bus_socket to allow access to the host dbus and a shared volume for holding the downloaded update bundle.

By default the path for holding the downloaded update bundles is assumed to be /data/selfupdates. If you wish to choose another download location then update the yaml file (lines 21 & 40) with your desired path.

There are also various port settings. You may need to adjust these values to match the setup that you are using.
