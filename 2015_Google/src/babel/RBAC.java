package com.google.l10n.security;

import com.google.appengine.api.users.User;

import java.net.InetAddress;

/**
 * <b>Purpose</b>: Security Implementor for Role Based Access Control
 - Contains all security objects
 - Relays call to appropriate object
 - Provides SAP, single access point authorization
 - Handles and relays FlushCache events
 */
class RBAC implements Security {
  private final IpAddressGuard ipAddressGuard;
  private final Authenticate authenticate;
  private final QueryRole resolver;
  private final AuthorizeOperation MAC;
  private final AuthorizeResource DAC;
  private final Iterable<FlushCache> notifyInitializeEvent;

  public static final RBAC build(Authenticate auth, QueryRole queryRole, AuthorizeOperation mac,
      AuthorizeResource dac, IpAddressGuard guard, Iterable<FlushCache> notify) {
    return new RBAC(auth, queryRole, mac, dac, guard, notify);
  }

  private RBAC(Authenticate a, QueryRole r, AuthorizeOperation mac, AuthorizeResource dac,
      IpAddressGuard guard, Iterable<FlushCache> notify) {
    this.ipAddressGuard = guard;
    this.authenticate = a;
    this.resolver = r;
    this.MAC = mac;
    this.DAC = dac;
    this.notifyInitializeEvent = notify;
  }

  @Override public boolean authenticate(User user) {
    return this.authenticate.authenticate(user);
  }

  @Override public final Role getRole(String principal) {
    return this.resolver.getRole(principal);
  }

  @Override public final boolean authorizeLocation(InetAddress ipAddress, String principal, 
      Operation operation) {
    return this.MAC.authorizeLocation(ipAddress, principal, operation);
  }

  @Override public final boolean authorize(InetAddress ipAddress, String principal, 
      Role role, Operation operation) {
    return this.MAC.authorize(ipAddress, principal, role, operation);
  }

  @Override public final boolean authorize(InetAddress ipAddress, String principal, 
      Role role, Resource resource) {
    return this.DAC.authorize(ipAddress, principal, role, resource);
  }

  @Override public boolean authorize(InetAddress ipAddress, User user, Operation operation) {
    return this.MAC.authorize(ipAddress, user, operation);
  }

  @Override public boolean authorize(InetAddress ipAddress, User user, Resource resource) {
    return this.DAC.authorize(ipAddress, user, resource);
  }

  @Override public boolean authorize(InetAddress ipAddress, User user, 
      Operation operation, Resource resource) {
    if (this.authenticate.authenticate(user)) {
      String principal = user.getEmail();
      Role role = this.resolver.getRole(principal);
      if (!role.isUnauthorized() && 
          this.MAC.authorizeLocation(ipAddress, user.getEmail(), operation) && 
          this.MAC.authorize(ipAddress, principal, role, operation) && 
          this.DAC.authorize(ipAddress, principal, role, resource)) {
        return true;
      }
    }
    return false;
  }

  @Override public Iterable<Resource> listAuthorizedResources(String principal, ResourceType resourceType) {
    return this.DAC.listAuthorizedResources(principal, resourceType);
  }

  @Override public boolean isBlocked(InetAddress ipAddress, String email) {
    return this.ipAddressGuard.isBlocked(ipAddress, email);
  }

  @Override public void flush() {
    for (FlushCache notify : this.notifyInitializeEvent) {
      notify.flush();
    }
  }

}
