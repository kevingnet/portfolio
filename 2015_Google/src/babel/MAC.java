package com.google.l10n.security;

import com.google.appengine.api.users.User;

import java.net.InetAddress;
import java.util.Map;

/**
 * <b>Purpose</b>: Authorizes a User to an Operation via Role
 - When this object is built it is given a list of operations and a list of objecst to query for
 roles
 - Caches Q&A
 */
class MAC extends Cached<String, Integer> implements AuthorizeOperation {
  private final QueryRole resolver;
  private final OperationIpAddressValidator operationIpAddressValidator;
  private final IpAddressGuard ipAddressGuard;
  private final Map<Operation, Role> permissions;

  public static final MAC build(QueryRole rr, Map<Operation, Role> perms, 
      OperationIpAddressValidator ipMap, IpAddressGuard ipGuard,
      Cache<String, Integer> cache) {
    return new MAC(rr, perms, ipMap, ipGuard, cache);
  }

  private MAC(QueryRole rr, Map<Operation, Role> perms, 
      OperationIpAddressValidator ipMap, IpAddressGuard ipGuard, 
      Cache<String, Integer> cache) {
    super(cache);
    this.resolver = rr;
    this.operationIpAddressValidator = ipMap;
    this.permissions = perms;
    this.ipAddressGuard = ipGuard;
  }

  @Override public boolean authorizeLocation(InetAddress ipAddress, String email, 
      com.google.l10n.security.Operation operation) {
    if (!this.operationIpAddressValidator.isValid(operation, ipAddress)) {
      this.ipAddressGuard.block(ipAddress, email);
      return false;
    }
    return true;
  }

  @Override public boolean authorize(InetAddress ipAddress, User user, 
      com.google.l10n.security.Operation operation) {
    String principal = user.getEmail();
    Role role = this.resolver.getRole(principal);
    return authorize(ipAddress, principal, role, operation);
  }

  @Override public final boolean authorize(InetAddress ipAddress, String principal, 
      Role role, com.google.l10n.security.Operation operation) {
    final Integer hash = hash(role, operation);
    if (exists(principal, hash)) {
      return true;
    }
    final Role required = this.permissions.get(operation);
    if (role.isManager() || role.matches(required)) {
      set(principal, hash);
      return true;
    }
    //this.ipAddressGuard.block(ipAddress, principal);
    return false;
  }

  private final static Integer hash(Role role, Operation operation) {
    final String data = String.valueOf(role.toInt()) + operation.toString();
    return data.hashCode();
  }

  @Override public Integer get(String hash) {
    return 0;
  }
}
