package com.google.l10n.security;

import com.google.appengine.api.users.User;
import com.google.common.collect.ImmutableList;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;

/**
 * <b>Purpose</b>: Authorizes a User to a Resource
 - Has a list of objects capable of answering the question weather a user is authorized a certain
 resource
 - Caches Q&A
 */
class DAC extends Cached<String, Integer> implements AuthorizeResource {
  private final QueryRole resolver;
  private final IpAddressGuard ipAddressGuard;
  private final ImmutableList<AuthorizeResource> authorizers;

  public static final DAC build(QueryRole rr, Iterable<AuthorizeResource> providers,
      IpAddressGuard ipGuard, Cache<String, Integer> cache) {
    return new DAC(rr, providers, ipGuard, cache);
  }

  private DAC(QueryRole rr, Iterable<AuthorizeResource> providers, 
      IpAddressGuard ipGuard, Cache<String, Integer> cache) {
    super(cache);
    this.resolver = rr;
    this.authorizers = ImmutableList.copyOf(providers);
    this.ipAddressGuard = ipGuard;
  }

  @Override public boolean authorize(InetAddress ipAddress, User user, Resource resource) {
    String principal = user.getEmail();
    Role role = this.resolver.getRole(principal);
    return authorize(ipAddress, principal, role, resource);
  }

  @Override public final boolean authorize(InetAddress ipAddress, String principal, 
      Role role, Resource resource) {
    final Integer hash = hash(role, resource);
    if (exists(principal, hash)) {
      return true;
    }
    for (AuthorizeResource auth : this.authorizers) {
      if (auth.authorize(ipAddress, principal, role, resource)) {
        set(principal, hash);
        return true;
      }
    }
    this.ipAddressGuard.block(ipAddress, principal);
    return false;
  }

  @Override public Iterable<Resource> listAuthorizedResources(String principal, ResourceType resourceType) {
    List<Resource> resources = new ArrayList<>();
    for (AuthorizeResource auth : this.authorizers) {
      for (Resource resource: auth.listAuthorizedResources(principal, resourceType)) {
        resources.add(resource);
      }
    }
    Iterable<Resource> iterable = resources;
    return iterable;
  }

  private final static Integer hash(Role role, Resource resource) {
    final String data = String.valueOf(role.toInt()) + resource.toString();
    return data.hashCode();
  }

  @Override public Integer get(String hash) {
    return 0;
  }
}
